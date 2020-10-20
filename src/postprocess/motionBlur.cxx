/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file motionBlur.cxx
 * @author lachbr
 * @date 2020-09-03
 *
 * Image-space motion blur.
 */

#include "motionBlur.h"
#include "mathutil_misc.h"
#include "clockObject.h"
#include "camera.h"
#include "orthographicLens.h"
#include "transformState.h"
#include "cardMaker.h"
#include "omniBoundingVolume.h"
#include "callbackNode.h"
#include "callbackObject.h"
#include "geomDrawCallbackData.h"
#include "graphicsStateGuardian.h"

ConfigVariableBool mat_motion_blur_enabled("mat_motion_blur_enabled", true);
ConfigVariableBool mat_motion_blur_forward_enabled("mat_motion_blur_forward_enabled", false);
ConfigVariableDouble mat_motion_blur_falling_min("mat_motion_blur_falling_min", 10.0);
ConfigVariableDouble mat_motion_blur_falling_max("mat_motion_blur_falling_max", 20.0);
ConfigVariableDouble mat_motion_blur_falling_intensity("mat_motion_blur_falling_intensity", 1.0);
ConfigVariableDouble mat_motion_blur_rotation_intensity("mat_motion_blur_rotation_intensity", 1.0);
ConfigVariableDouble mat_motion_blur_strength("mat_motion_blur_strength", 1.0);
ConfigVariableDouble mat_motion_blur_percent_of_screen_max("mat_motion_blur_percent_of_screen_max", 4.0);

class MotionBlurCallback : public CallbackObject {
public:
  ALLOC_DELETED_CHAIN(MotionBlurCallback);

  MotionBlurCallback(MotionBlur *mb) :
    _mb(mb)
  {}

  virtual void do_callback(CallbackData *data) {
    _mb->draw(data);
  }

private:
  MotionBlur *_mb;
};

MotionBlur::
MotionBlur(GraphicsOutput *output) {
  _output = output;
  _quad = nullptr;
  _quad_state = nullptr;
  _framebuffer_texture = new Texture;
  _framebuffer_texture->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_srgb);
  _framebuffer_texture->set_magfilter(SamplerState::FT_linear);
  _framebuffer_texture->set_minfilter(SamplerState::FT_linear);
  _framebuffer_texture->set_wrap_u(SamplerState::WM_clamp);
  _framebuffer_texture->set_wrap_v(SamplerState::WM_clamp);
  _last_time_update = 0.0f;
  _previous_pitch = 0.0f;
  _previous_yaw = 0.0f;
  _no_rotational_motion_blur_until = 0.0f;
  _motion_blur_params.resize(1);
  _consts.resize(1);
}

void MotionBlur::
shutdown() {
  _output = nullptr;
  _quad = nullptr;
  _quad_state = nullptr;
  _framebuffer_texture = nullptr;
  _scene_camera.clear();
  _camera.remove_node();
  _np.remove_node();
}

void MotionBlur::
set_scene_camera(const NodePath &camera) {
  _scene_camera = camera;
}

const NodePath &MotionBlur::
get_camera() const {
  return _camera;
}

void MotionBlur::
draw(CallbackData *data) {
  GeomDrawCallbackData *geom_cbdata;
  DCAST_INTO_V(geom_cbdata, data);
  GraphicsStateGuardian *gsg;
  DCAST_INTO_V(gsg, geom_cbdata->get_gsg());

  Thread *thread = Thread::get_current_thread();

  geom_cbdata->set_lost_state(false);

  CullableObject *obj = geom_cbdata->get_object();

  // Copy the current framebuffer into the texture that the motion blur
  // shader will sample. This is done because the motion blur quad writes
  // to the same framebuffer that we are sampling from.
  gsg->framebuffer_copy_to_texture(_framebuffer_texture, 0, -1,
    gsg->get_current_display_region(), RenderBuffer(gsg, RenderBuffer::T_color));

  gsg->set_state_and_transform(_quad_state, obj->_internal_transform);

  CPT(Geom) munged_geom = _quad;
  CPT(GeomVertexData) munged_data = _quad->get_vertex_data();
  gsg->get_geom_munger(_quad_state, thread)->
    munge_geom(munged_geom, munged_data, true, thread);

  // Draw it!
  munged_geom->draw(gsg, munged_data, true, thread);
}

void MotionBlur::
setup() {
  CardMaker cm("motion-blur-card");
  cm.set_frame_fullscreen_quad();
  NodePath np = NodePath(cm.generate());
  np.set_depth_write(false);
  np.set_depth_test(false);
  np.set_shader(
    Shader::load(
      Shader::SL_GLSL,
      "shaders/postprocess/motion_blur.vert.glsl",
      "shaders/postprocess/motion_blur.frag.glsl"
    )
  );
  np.set_shader_input(ShaderInput("motionBlurParams", _motion_blur_params));
  np.set_shader_input(ShaderInput("texSampler", _framebuffer_texture));
  np.set_shader_input(ShaderInput("consts", _consts));

  // Pull out the geom and state, we will use this to render
  // during our draw callback
  GeomNode *gn = DCAST(GeomNode, np.node());
  _quad = gn->get_geom(0);
  _quad_state = np.get_state();

  // Set up our callback node with a draw callback.
  // The draw callback will copy the current framebuffer and draw
  // the motion blur quad.
  PT(CallbackNode) node = new CallbackNode("motionblur-callback");
  node->set_draw_callback(new MotionBlurCallback(this));
  node->set_bounds(new OmniBoundingVolume);
  _np = NodePath(node);

  PT(OrthographicLens) lens = new OrthographicLens;
  lens->set_film_size(2, 2);
  lens->set_film_offset(0, 0);
  lens->set_near_far(-1000, 1000);
  PT(Camera) camera = new Camera("motion-blur-camera");
  camera->set_lens(lens);
  camera->set_bounds(new OmniBoundingVolume);
  _camera = _np.attach_new_node(camera);
}

void MotionBlur::
update() {
  float motion_blur_rotation_intensity = mat_motion_blur_rotation_intensity.get_value() * 0.15f;
  float motion_blur_roll_intensity = 0.3f;
  float motion_blur_falling_intensity = mat_motion_blur_falling_intensity.get_value();
  float motion_blur_falling_min = mat_motion_blur_falling_min.get_value();
  float motion_blur_falling_max = mat_motion_blur_falling_max.get_value();
  float motion_blur_global_strength = mat_motion_blur_strength.get_value();

  ClockObject *clock = ClockObject::get_global_clock();

  float time_elapsed = clock->get_frame_time() - _last_time_update;

  NodePath camera_path = _scene_camera;
  Camera *camera = DCAST(Camera, camera_path.node());
  Lens *lens = camera->get_lens();
  CPT(TransformState) transform = camera_path.get_net_transform();
  const LVecBase3 &hpr = transform->get_hpr();
  const LPoint3 &pos = transform->get_pos();
  const LQuaternion &quat = transform->get_norm_quat();

  // Get current pitch & wrap to +-180
  float current_pitch = hpr[1];
  while (current_pitch > 180.0f) {
    current_pitch -= 360.0f;
  }
  while (current_pitch < -180.0f) {
    current_pitch += 360.0f;
  }

  // Get current yaw & wrap to +-180
  float current_yaw = hpr[0];
  while (current_yaw > 180.0f) {
    current_yaw -= 360.0f;
  }
  while (current_yaw < -180.0f) {
    current_yaw += 360.0f;
  }

  // Get current basis vectors
  LVector3 current_side_vec = quat.get_right();
  LVector3 current_forward_vec = quat.get_forward();

  // Evaluate change in position to determine if we need to update
  LVector3 pos_change = pos - _previous_position;
  float change_length = pos_change.length();

  if ((change_length > 1.875f) && (time_elapsed >= 0.5f)) {
    // If we moved a far distance in one frame and more than
    // half a second elapsed, disable motion blur this frame.
    _motion_blur_params[0].set(0, 0, 0, 0);

  } else if (time_elapsed >= (1.0f / 15.0f)) {
    // If slower than 15 fps, don't motion blur
    _motion_blur_params[0].set(0, 0, 0, 0);

  } else if (change_length > 3.125f) {
    // We moved a far distance in a frame, use the same motion blur as last frame
    // because I think we just went through a portal (should we ifdef this behavior?)
    _no_rotational_motion_blur_until = clock->get_frame_time() + 1.0f; // Wait a second until the portal craziness calms down

  } else {
    // Normal update path

    // Compute horizontal and vertical fov
    const LVecBase2 &fov = lens->get_fov();
    float horizontal_fov = fov[0];
    float vertical_fov = fov[1];

    // Forward motion blur
    float view_dot_motion = current_forward_vec.dot(pos_change);
    if (mat_motion_blur_forward_enabled) { // Want forward and falling
      _motion_blur_params[0][2] = view_dot_motion;
    } else { // Falling only
      _motion_blur_params[0][2] = view_dot_motion * std::fabs(current_forward_vec[2]); // Only want this if we're looking up or down
    }

    // Yaw (compensate for circle strafe)
    float side_dot_motion = current_side_vec.dot(pos_change);
    float yaw_diff_original = _previous_yaw - current_yaw;
    if ((_previous_yaw - current_yaw > 180.0f || _previous_yaw - current_yaw < -180.0f) &&
        (_previous_yaw + current_yaw > -180.0f && _previous_yaw + current_yaw < 180.0f)) {
      yaw_diff_original = _previous_yaw + current_yaw;
    }

    float yaw_diff_adjusted = yaw_diff_original + (side_dot_motion / 3.0f);

    // Make sure the adjustment only lessens the effect, not magnify it or reverse it
    if (yaw_diff_original < 0.0f) {
      yaw_diff_adjusted = clamp(yaw_diff_adjusted, yaw_diff_original, 0.0f);
    } else {
      yaw_diff_adjusted = clamp(yaw_diff_adjusted, 0.0f, yaw_diff_original);
    }

    // Use pitch to dampen yaw
    float undampened_yaw = yaw_diff_adjusted / horizontal_fov;
    _motion_blur_params[0][0] = undampened_yaw * (1.0f - (std::fabs(current_pitch) / 90.0f)); // Dampen horizontal yaw blur based on pitch

    // Pitch (Compenstate for forward motion)
    float pitch_compensate_mask = 1.0f - ((1.0f - std::fabs(current_forward_vec[2])) * (1.0f - std::fabs(current_forward_vec[2])));
    float pitch_diff_original = _previous_pitch - current_pitch;
    float pitch_diff_adjusted = pitch_diff_original;

    if (current_pitch > 0.0f) {
      pitch_diff_adjusted = pitch_diff_original - ((view_dot_motion / 2.0f) * pitch_compensate_mask);
    } else {
      pitch_diff_adjusted = pitch_diff_original + ((view_dot_motion / 2.0f) * pitch_compensate_mask);
    }

    // Make sure the adjustment only lessens the effect, not magnify it or reverse it
    if (pitch_diff_original < 0.0f) {
      pitch_diff_adjusted = clamp(pitch_diff_adjusted, pitch_diff_original, 0.0f);
    } else {
      pitch_diff_adjusted = clamp(pitch_diff_adjusted, 0.0f, pitch_diff_original);
    }

    _motion_blur_params[0][1] = pitch_diff_adjusted / vertical_fov;

    // Roll (Enabled when we're looking down and yaw changes)
    _motion_blur_params[0][3] = undampened_yaw; // Roll starts out as undampened yaw intensity and is then scaled by pitch
    _motion_blur_params[0][3] *= (std::fabs(current_pitch) / 90.0f) * (std::fabs(current_pitch) / 90.0f) * (std::fabs(current_pitch) / 90.0f); // Dampen roll based on pitch^3

    // Time-adjust falling effect until we can do something smarter
    if (time_elapsed > 0.0f) {
      _motion_blur_params[0][2] /= time_elapsed * 30.0f; // 1/30th of a second?
    } else {
      _motion_blur_params[0][2] = 0.0f;
    }

    // Scale and bias values after time adjustment
    _motion_blur_params[0][2] = clamp((std::fabs(_motion_blur_params[0][2]) - motion_blur_falling_min) / (motion_blur_falling_max - motion_blur_falling_min),
                                        0.0f, 1.0f) * (_motion_blur_params[0][2] >= 0.0f ? 1.0f : -1.0f);
    _motion_blur_params[0][2] /= 30.0f; // To counter-adjust for time adjustment above

    // Apply intensity
    _motion_blur_params[0][0] *= motion_blur_rotation_intensity * motion_blur_global_strength;
    _motion_blur_params[0][1] *= motion_blur_rotation_intensity * motion_blur_global_strength;
    _motion_blur_params[0][2] *= motion_blur_falling_intensity * motion_blur_global_strength;
    _motion_blur_params[0][3] *= motion_blur_roll_intensity * motion_blur_global_strength;

    // Dampen motion blur from 100%-0% as fps drops from 50fps-30fps
    static float slow_fps = 30.0f;
    static float fast_fps = 50.0f;
    float current_fps = (time_elapsed > 0.0f) ? (1.0f / time_elapsed) : 0.0f;
    float dampen_factor = clamp(((current_fps - slow_fps) / (fast_fps - slow_fps)), 0.0f, 1.0f);

    _motion_blur_params[0][0] *= dampen_factor;
    _motion_blur_params[0][1] *= dampen_factor;
    _motion_blur_params[0][2] *= dampen_factor;
    _motion_blur_params[0][3] *= dampen_factor;
  }

  // Zero out blur if still in that time window
  if (clock->get_frame_time() < _no_rotational_motion_blur_until) {
    // Zero out rotational blur but leave forward/falling blur alone
    _motion_blur_params[0][0] = 0.0f; // X
    _motion_blur_params[0][1] = 0.0f; // Y
    _motion_blur_params[0][3] = 0.0f; // Roll
  } else {
    _no_rotational_motion_blur_until = 0.0f;
  }

  Texture *src_texture = _framebuffer_texture;
  int texture_height = src_texture->get_y_size();

  // Percent of screen clamp
  _consts[0][0] = mat_motion_blur_percent_of_screen_max.get_value() / 100.0f;

  // Quality based on screen resolution height
  int quality = 1;
  if (texture_height >= 1080) { // 1080p or higher
    quality = 3;
  } else if (texture_height >= 720) {
    // 720p or higher
    quality = 2;
  } else {
    // Lower resolution than 720p
    quality = 1;
  }

  if (std::fabs(_motion_blur_params[0][0]) + std::fabs(_motion_blur_params[0][1]) +
      std::fabs(_motion_blur_params[0][2]) + std::fabs(_motion_blur_params[0][3]) == 0.0f) {
    // No motion blur this frame, so force quality to 0
    quality = 0;
  }

  _consts[0][1] = quality;

  // Store current frame for next frame
  _previous_position = pos;
  _previous_pitch = current_pitch;
  _previous_yaw = current_yaw;
  _last_time_update = clock->get_frame_time();
}
