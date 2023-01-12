/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file motionBlur.cxx
 * @author brian
 * @date 2020-09-03
 *
 * Image-space motion blur.
 */

#include "motionBlur.h"
#include "postProcessPass.h"
#include "postProcess.h"
#include "mathutil_misc.h"
#include "clockObject.h"
#include "camera.h"

IMPLEMENT_CLASS(MotionBlur);

ConfigVariableBool mat_motion_blur_enabled("mat_motion_blur_enabled", true);
ConfigVariableBool mat_motion_blur_forward_enabled("mat_motion_blur_forward_enabled", false);
ConfigVariableDouble mat_motion_blur_falling_min("mat_motion_blur_falling_min", 10.0);
ConfigVariableDouble mat_motion_blur_falling_max("mat_motion_blur_falling_max", 20.0);
ConfigVariableDouble mat_motion_blur_falling_intensity("mat_motion_blur_falling_intensity", 1.0);
ConfigVariableDouble mat_motion_blur_rotation_intensity("mat_motion_blur_rotation_intensity", 0.15);
ConfigVariableDouble mat_motion_blur_roll_intensity("mat_motion_blur_roll_intensity", 0.3);
ConfigVariableDouble mat_motion_blur_strength("mat_motion_blur_strength", 1.0);
ConfigVariableDouble mat_motion_blur_percent_of_screen_max("mat_motion_blur_percent_of_screen_max", 4.0);

class MotionBlurPass : public PostProcessPass {
  DECLARE_CLASS(MotionBlurPass, PostProcessPass);

public:
  MotionBlurPass(PostProcess *pp, Texture *fb_tex);
  virtual void on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg) override;

private:
  Texture *_fb_tex;
};

IMPLEMENT_CLASS(MotionBlurPass);

/**
 *
 */
MotionBlurPass::
MotionBlurPass(PostProcess *pp, Texture *fb_tex) :
  PostProcessPass(pp, "motion-blur-pass"),
  _fb_tex(fb_tex)
{
  // We are a display region on top of the scene pass.  The only reason this is
  // done is so we can exclude view models from motion blur.
  set_window_layer(true, pp->get_scene_pass()->get_buffer(), 1);
}

/**
 *
 */
void MotionBlurPass::
on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg) {
  // Copy the current scene framebuffer to a texture that we can motion blur,
  // to exclude view models from motion blur.
  gsg->framebuffer_copy_to_texture(_fb_tex, 0, -1,
    gsg->get_current_display_region(), RenderBuffer(gsg, RenderBuffer::T_color));

  // Now draw the motion blur quad.
  PostProcessPass::on_draw(cbdata, gsg);
}

MotionBlur::
MotionBlur(PostProcess *pp) :
  PostProcessEffect(pp, "motion-blur") {

  _framebuffer_texture = new Texture("motion-blur-fb-copy");
  _framebuffer_texture->set_format(Texture::F_rgba16);
  _framebuffer_texture->set_minfilter(SamplerState::FT_linear);
  _framebuffer_texture->set_magfilter(SamplerState::FT_linear);
  _framebuffer_texture->set_wrap_u(SamplerState::WM_clamp);
  _framebuffer_texture->set_wrap_v(SamplerState::WM_clamp);

  _last_time_update = 0.0f;
  _previous_pitch = 0.0f;
  _previous_yaw = 0.0f;
  _no_rotational_motion_blur_until = 0.0f;
  _motion_blur_params.resize(1);
  _consts.resize(1);

  PT(MotionBlurPass) pass = new MotionBlurPass(pp, _framebuffer_texture);
  pass->setup();

  NodePath np = pass->get_quad();
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

  add_pass(pass);
}

void MotionBlur::
set_scene_camera(const NodePath &camera) {
  _scene_camera = camera;
}

void MotionBlur::
update() {
  PostProcessEffect::update();

  nassertv(!_scene_camera.is_empty());

  float motion_blur_rotation_intensity = mat_motion_blur_rotation_intensity.get_value();
  float motion_blur_roll_intensity = mat_motion_blur_roll_intensity.get_value();
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
  LVecBase3 hpr = transform->get_hpr();
  LPoint3 pos = transform->get_pos();
  LQuaternion quat = transform->get_quat();

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

  float x_blur, y_blur, roll_blur, forward_blur;

  if ((change_length > 30.0f) && (time_elapsed >= 0.5f)) {
    // If we moved a far distance in one frame and more than
    // half a second elapsed, disable motion blur this frame.
    x_blur = y_blur = roll_blur = forward_blur = 0.0f;

  } else if (time_elapsed >= (1.0f / 15.0f)) {
    // If slower than 15 fps, don't motion blur
    x_blur = y_blur = roll_blur = forward_blur = 0.0f;

  } else if (change_length > 50.0f) {
    // We moved a far distance in a frame, use the same motion blur as last frame
    // because I think we just went through a portal (should we ifdef this behavior?)
    _no_rotational_motion_blur_until = clock->get_frame_time() + 1.0f; // Wait a second until the portal craziness calms down
    x_blur = _motion_blur_params[0][0];
    y_blur = _motion_blur_params[0][1];
    forward_blur = _motion_blur_params[0][2];
    roll_blur = _motion_blur_params[0][3];

  } else {
    // Normal update path

    // Compute horizontal and vertical fov
    const LVecBase2 &fov = lens->get_fov();
    float horizontal_fov = fov[0];
    float vertical_fov = fov[1];

    // Forward motion blur
    float view_dot_motion = current_forward_vec.dot(pos_change);
    if (mat_motion_blur_forward_enabled) { // Want forward and falling
      forward_blur = view_dot_motion;
    } else { // Falling only
      forward_blur = view_dot_motion * std::fabs(current_forward_vec[2]); // Only want this if we're looking up or down
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
      yaw_diff_adjusted = std::clamp(yaw_diff_adjusted, yaw_diff_original, 0.0f);
    } else {
      yaw_diff_adjusted = std::clamp(yaw_diff_adjusted, 0.0f, yaw_diff_original);
    }

    // Use pitch to dampen yaw
    float undampened_yaw = yaw_diff_adjusted / horizontal_fov;
    x_blur = undampened_yaw * (1.0f - (std::fabs(current_pitch) / 90.0f)); // Dampen horizontal yaw blur based on pitch

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
      pitch_diff_adjusted = std::clamp(pitch_diff_adjusted, pitch_diff_original, 0.0f);
    } else {
      pitch_diff_adjusted = std::clamp(pitch_diff_adjusted, 0.0f, pitch_diff_original);
    }

    y_blur = pitch_diff_adjusted / vertical_fov;

    // Roll (Enabled when we're looking down and yaw changes)
    roll_blur = undampened_yaw; // Roll starts out as undampened yaw intensity and is then scaled by pitch
    roll_blur *= (std::fabs(current_pitch) / 90.0f) * (std::fabs(current_pitch) / 90.0f) * (std::fabs(current_pitch) / 90.0f); // Dampen roll based on pitch^3

    // Time-adjust falling effect until we can do something smarter
    if (time_elapsed > 0.0f) {
      forward_blur /= time_elapsed * 30.0f; // 1/30th of a second?
    } else {
      forward_blur = 0.0f;
    }

    // Scale and bias values after time adjustment
    forward_blur = std::clamp((std::fabs(forward_blur) - motion_blur_falling_min) / (motion_blur_falling_max - motion_blur_falling_min),
                                        0.0f, 1.0f) * (forward_blur >= 0.0f ? 1.0f : -1.0f);
    forward_blur /= 30.0f; // To counter-adjust for time adjustment above

    // Apply intensity
    x_blur *= motion_blur_rotation_intensity * motion_blur_global_strength;
    y_blur *= motion_blur_rotation_intensity * motion_blur_global_strength;
    forward_blur *= motion_blur_falling_intensity * motion_blur_global_strength;
    roll_blur *= motion_blur_roll_intensity * motion_blur_global_strength;

    // Dampen motion blur from 100%-0% as fps drops from 50fps-30fps
    static float slow_fps = 30.0f;
    static float fast_fps = 50.0f;
    float current_fps = (time_elapsed > 0.0f) ? (1.0f / time_elapsed) : 0.0f;
    float dampen_factor = std::clamp(((current_fps - slow_fps) / (fast_fps - slow_fps)), 0.0f, 1.0f);

    x_blur *= dampen_factor;
    y_blur *= dampen_factor;
    forward_blur *= dampen_factor;
    roll_blur *= dampen_factor;
  }

  // Zero out blur if still in that time window
  if (clock->get_frame_time() < _no_rotational_motion_blur_until) {
    // Zero out rotational blur but leave forward/falling blur alone
    x_blur = 0.0f; // X
    y_blur = 0.0f; // Y
    roll_blur = 0.0f; // Roll
  } else {
    _no_rotational_motion_blur_until = 0.0f;
  }

  int texture_height = _pp->get_output()->get_y_size();

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

  if (std::fabs(x_blur) + std::fabs(y_blur) +
      std::fabs(forward_blur) + std::fabs(roll_blur) == 0.0f) {
    // No motion blur this frame, so force quality to 0
    quality = 0;
  }

  _motion_blur_params[0][0] = x_blur;
  _motion_blur_params[0][1] = y_blur;
  _motion_blur_params[0][2] = forward_blur;
  _motion_blur_params[0][3] = roll_blur;

  _consts[0][1] = quality;

  // Store current frame for next frame
  _previous_position = pos;
  _previous_pitch = current_pitch;
  _previous_yaw = current_yaw;
  _last_time_update = clock->get_frame_time();
}
