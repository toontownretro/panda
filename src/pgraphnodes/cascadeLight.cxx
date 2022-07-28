/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cascadeLight.h
 * @author lachbr
 * @date 2020-10-26
 */

#include "cascadeLight.h"
#include "orthographicLens.h"
#include "clockObject.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "look_at.h"
#include "shaderAttrib.h"
#include "omniBoundingVolume.h"
#include "lightAttrib.h"
#include "boundingSphere.h"
#include "cullFaceAttrib.h"
#include "boundingBox.h"

TypeHandle CascadeLight::_type_handle;

/**
 *
 */
CascadeLight::
CascadeLight(const std::string &name) :
  DirectionalLight(name),
  _scene_camera(NodePath()) {

  _csm_distance = csm_distance;
  _sun_distance = csm_sun_distance;
  _log_factor = csm_log_factor;
  _border_bias = csm_border_bias;
  _fixed_film_size = csm_fixed_film_size;
  _num_cascades = csm_num_cascades;

  _last_update_frame = -1;

  // Make sure we render the shadow scene using the specialized CSMDepth
  // shader.
  CPT(RenderState) state = get_initial_state();
  //state = state->set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_counter_clockwise), 100);
  state = state->set_attrib(ShaderAttrib::make("CSMDepth"), 100);
  set_initial_state(state);

  // Don't perform any view-frustum culling when rendering to the cascades.
  // This is bad.  I would like to implement a way to compute a tight bounding
  // volume around all objects that may cast shadows into the main view.
  set_cull_bounds(new OmniBoundingVolume);

  get_lens()->set_film_size(100, 100);
  get_lens()->set_near_far(1, 400);

  // Make sure we are always visited during the Cull traversal regardless of if
  // the light node is outside the viewing frustum.
  set_bounds(new OmniBoundingVolume);
  set_final(true);

  // We need a cull callback to update the cascades when we are visited during
  // the cull traversal.
  set_cull_callback();
  set_renderable();
}

/**
 * Creates the shadow map texture.  Can be overridden.
 */
void CascadeLight::
setup_shadow_map() {
  if (_cascades.size() != _num_cascades) {
    // The number of cascades changed.  Need to recreate them.
    setup_cascades();
  }

  if (_shadow_map != nullptr &&
      _shadow_map->get_x_size() == _sb_size[0] &&
      _shadow_map->get_y_size() == _sb_size[1] &&
      _shadow_map->get_z_size() == _num_cascades) {

    // Nothing actually changed; we don't need to recreate the shadow map.
    return;
  }

  if (_shadow_map == nullptr) {
    _shadow_map = new Texture(get_name());
  }

#if 0
  // The specified shadow map size is the total size of the atlas
  // for all cascades.  Each cascade takes up the same space in the
  // atlas, which is size / (num_cascades / 2).

  int width_per_cascade = _sb_size[0] / (_num_cascades / 2);
  int height_per_cascade = _sb_size[1] / (_num_cascades / 2);
  PN_stdfloat wscale = width_per_cascade / (PN_stdfloat)_sb_size[0];
  PN_stdfloat hscale = height_per_cascade / (PN_stdfloat)_sb_size[1];

  std::cout << "size per cascade: " << width_per_cascade << "x" << height_per_cascade << "\n";

  // Pack cascades onto grid.
  int curr_x = 0;
  int curr_y = 0;
  for (int i = 0; i < _num_cascades; ++i) {
    if (curr_x >= _sb_size[0]) {
      // Go to next row.
      curr_y += height_per_cascade;
      curr_x = 0;
      nassertv(curr_y < _sb_size[1]);
    }

    _cascades[i].atlas_mins.set(curr_x / (PN_stdfloat)_sb_size[0],
                                curr_y / (PN_stdfloat)_sb_size[1]);
    _cascades[i].atlas_scale.set(wscale, hscale);
    _cascades[i].atlas_maxs = _cascades[i].atlas_mins + _cascades[i].atlas_scale;

    std::cout << "cascade " << i << "\n";
    std::cout << "mins: " << _cascades[i].atlas_mins << "\n";
    std::cout << "maxs: " << _cascades[i].atlas_maxs << "\n";
    std::cout << "scale: " << _cascades[i].atlas_scale << "\n";

    // Go to next column.
    curr_x += width_per_cascade;
  }
#endif

  _shadow_map->setup_2d_texture_array(_sb_size[0], _sb_size[1], _num_cascades,
                                      Texture::T_unsigned_byte, Texture::F_depth_component);
  _shadow_map->set_clear_color(LColor(1));
  _shadow_map->set_wrap_u(SamplerState::WM_clamp);
  _shadow_map->set_wrap_v(SamplerState::WM_clamp);
  _shadow_map->set_border_color(LColor(1));
  _shadow_map->set_minfilter(SamplerState::FT_shadow);
  _shadow_map->set_magfilter(SamplerState::FT_shadow);
}

/**
 *
 */
void CascadeLight::
update(const NodePath &root) {
  //std::cout << "Cascade callback\n";
  if (!_shadow_caster) {
    // We aren't even casting shadows.  Don't do anything.
    return;
  }

  ClockObject *global_clock = ClockObject::get_global_clock();
  int frame = global_clock->get_frame_count();
  //if (frame == _last_update_frame) {
  //  // We already updated this frame.  Don't do anything.
  //  return true;
  //}

  _last_update_frame = frame;

  //std::cout << "update cascades, we have " << _cascades.size() << "\n";

  // Get the camera node transform
  nassertv(!_scene_camera.is_empty());
  NodePath cam_np = _scene_camera.get_node_path();

  const LMatrix4 &transform = cam_np.get_net_transform()->get_mat();

  // Get Camera and Lens pointers
  Camera *cam = DCAST(Camera, cam_np.node());
  nassertv(cam != nullptr);
  Lens *lens = cam->get_lens();

  // Extract near and far points of scene camera.
  lens->extrude(LPoint2(-1, 1), _curr_near_points[CO_ul], _curr_far_points[CO_ul]);
  lens->extrude(LPoint2(1, 1), _curr_near_points[CO_ur], _curr_far_points[CO_ur]);
  lens->extrude(LPoint2(-1, -1), _curr_near_points[CO_ll], _curr_far_points[CO_ll]);
  lens->extrude(LPoint2(1, -1), _curr_near_points[CO_lr], _curr_far_points[CO_lr]);

  // Construct matrix to project points to world space
  LMatrix4 mvp = transform * lens->get_view_mat();

  // Project all points into world space
  for (size_t i = 0; i < 4; i++) {
    LPoint4 ws_near = mvp.xform(_curr_near_points[i]);
    LPoint4 ws_far = mvp.xform(_curr_far_points[i]);
    _curr_near_points[i] = ws_near.get_xyz();
    _curr_far_points[i] = ws_far.get_xyz();
  }

  // Do the actual PSSM
  compute_pssm_splits(transform, _csm_distance / lens->get_far(), cam, root);
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool CascadeLight::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  update(trav->get_scene()->get_scene_root());
  return true;
}

/**
 * Creates a Lens for each cascade.
 */
void CascadeLight::
setup_cascades() {
  // First, clear all the existing cascades.
  for (size_t i = 0; i < _cascades.size(); i++) {
    Cascade &c = _cascades[i];
    if (!c.node.is_empty()) {
      c.node.remove_node();
    }
  }

  _cascades.resize(_num_cascades);

  CDWriter cdata(_cycler);
  cdata->_cascade_mvps.resize(_num_cascades);
  cdata->_cascade_nearfar.resize(_num_cascades);

  _max_film_sizes.resize(_num_cascades);

  for (int i = 0; i < _num_cascades; i++) {
    Cascade &c = _cascades[i];
    c.lens = new OrthographicLens;
    std::ostringstream ss;
    ss << "cascade" << i;
    c.node = NodePath(ss.str());
    c.node.reparent_to(NodePath(this));
    cdata->_cascade_mvps[i] = LMatrix4::ident_mat();
    cdata->_cascade_nearfar[i] = LVecBase2(0, 1);
    _max_film_sizes[i] = LVecBase2(0);
  }
}

/**
 *
 */
static LVecBase3
get_snap_offset(const LMatrix4& mat, size_t resolution) {
  // Transform origin to camera space
  LPoint4 base_point = mat.get_row(3) * 0.5 + 0.5;

  // Compute the snap offset
  float texel_size = 1.0 / (float)(resolution);
  float offset_x = fmod(base_point.get_x(), texel_size);
  float offset_y = fmod(base_point.get_y(), texel_size);

  // Reproject the offset back, for that we need the inverse MVP
  LMatrix4 inv_mat(mat);
  inv_mat.invert_in_place();
  LVecBase3 new_base_point = inv_mat.xform_point(LVecBase3(
      (base_point.get_x() - offset_x) * 2.0 - 1.0,
      (base_point.get_y() - offset_y) * 2.0 - 1.0,
      base_point.get_z() * 2.0 - 1.0
    ));
  return -new_base_point;
}

/**
 * Computes the PSSM splits.
 */
void CascadeLight::
compute_pssm_splits(const LMatrix4 &transform, float max_distance,
                    Camera *main_cam, const NodePath &root) {

  // CSM distance should never be smaller than the camera far plane.
  nassertv(max_distance <= 1.0f);

  float filmsize_bias = 1.0f + _border_bias;

  NodePath light_np = NodePath(this);

  LPoint3 min_point(FLT_MAX);
  LPoint3 max_point(FLT_MIN);

  // Compute the positions of all cascades
  for (int i = 0; i < _num_cascades; i++) {
    Cascade &c = _cascades[i];

    float split_start = get_split_start(i) * max_distance;
    float split_end = get_split_start(i + 1) * max_distance;

    LVecBase3 start_points[4];
    LVecBase3 end_points[4];
    LVecBase3 proj_points[8];

    // Get split bounding box, and collect all points which define the frustum
    for (int k = 0; k < 4; k++) {
      start_points[k] = get_interpolated_point((CoordinateOrigin)k, split_start);
      end_points[k] = get_interpolated_point((CoordinateOrigin)k, split_end);

      proj_points[k] = start_points[k];
      proj_points[k + 4] = end_points[k];
    }

    // Compute approximate split mid point
    LPoint3 split_mid(0);
    for (int k = 0; k < 4; k++) {
      split_mid += start_points[k];
      split_mid += end_points[k];
    }
    split_mid /= 8.0;

    LVector3 light_vector = root.get_relative_vector(light_np, -get_direction());
    //std::cout << "light direction " << light_vector << "\n";
    LPoint3 cam_start = split_mid + light_vector * _sun_distance;

    // Reset the film size, offset, and far plane
    c.lens->set_film_size(1, 1);
    c.lens->set_film_offset(0, 0);
    c.lens->set_near_far(1, 100);

    // Find a good initial position
    c.node.set_pos(root, cam_start);
    c.node.look_at(root, split_mid);

    LVecBase3 best_min_extent, best_max_extent;

    // Find the minimum and maximum extens of the points
    CPT(TransformState) to_local = root.get_transform(c.node);
    calc_min_max_extents(best_min_extent, best_max_extent, to_local->get_mat(), proj_points, c);

    // Find the film size to cover all points
    LVecBase2 film_size, film_offset;
    get_film_properties(film_size, film_offset, best_min_extent, best_max_extent);

    if (_fixed_film_size) {
      // In case we use a fixed film size, store the maximum film size, and
      // only change the film size if a new maximum is there
      if (_max_film_sizes[i][0] < film_size[0]) {
        _max_film_sizes[i][0] = film_size[0];
      }
      if (_max_film_sizes[i][1] < film_size[1]) {
        _max_film_sizes[i][1] = film_size[1];
      }

      c.lens->set_film_size(_max_film_sizes[i] * filmsize_bias);
    } else {
      // If we don't use a fixed film size, we can just set the film size
      // on the lens.
      c.lens->set_film_size(film_size * filmsize_bias);
    }

    // Compute a new film offset
    c.lens->set_film_offset(film_offset);

    {
      CDWriter cdata(_cycler);
      cdata->_cascade_nearfar[i] = LVecBase2(best_min_extent[2],
                                             best_max_extent[2]);
    }
    c.lens->set_near_far(10.0f, _sun_distance * 2);

   if (i == 0) {
      // Reflect the first cascade on the lens of the camera... idk.
     get_lens()->set_film_size(c.lens->get_film_size());
     get_lens()->set_film_offset(c.lens->get_film_offset());
     get_lens()->set_near_far(c.lens->get_near(), c.lens->get_far());
     get_lens()->set_view_mat(c.node.get_mat());
    }

    // Compute the cascade MVP
    LMatrix4 mvp;
    compute_mvp(i, mvp, to_local);

    // Prevent flickering.
    LPoint3 snap_offset = get_snap_offset(mvp, _sb_size[0]);
    c.node.set_pos(c.node.get_pos() + snap_offset);
    to_local = root.get_transform(c.node);
    compute_mvp(i, mvp, to_local);

    // Store MVP in CData.
    {
      CDWriter cdata(_cycler);
      cdata->_cascade_mvps[i] = mvp;
    }

    PT(BoundingHexahedron) bounds = DCAST(BoundingHexahedron, c.lens->make_bounds());
    for (int j = 0; j < bounds->get_num_points(); j++) {
      LPoint3 b_point = bounds->get_point(j);
      // Move into space of the light node itself.
      b_point = c.node.get_mat().xform_point(b_point);
      min_point[0] = std::min(b_point[0], min_point[0]);
      min_point[1] = std::min(b_point[1], min_point[1]);
      min_point[2] = std::min(b_point[2], min_point[2]);
      max_point[0] = std::max(b_point[0], max_point[0]);
      max_point[1] = std::max(b_point[1], max_point[1]);
      max_point[2] = std::max(b_point[2], max_point[2]);
    }
  }

  // Use a bounding box that encloses the frusta of each cascade lens.
  set_cull_bounds(new BoundingBox(min_point, max_point));
  set_bounds(get_cull_bounds());
}

/**
 * Calculates the minimum and maximum screen coordinates of a cascade.
 */
void CascadeLight::
calc_min_max_extents(LVecBase3 &mins, LVecBase3 &maxs, const LMatrix4 &transform,
                     LVecBase3 *const proj_points, const Cascade &c) {
  mins.fill(1e10);
  maxs.fill(-1e10);
  LPoint2 screen_points[8];

  // Now project all points to the screen space of the current camera and also
  // find the minimum and maximum extents
  for (int i = 0; i < 8; i++) {
    LVecBase4 point(proj_points[i], 1);
    LPoint4 proj_point = transform.xform(point);
    LPoint3 proj_point_3d = proj_point.get_xyz();

    c.lens->project(proj_point_3d, screen_points[i]);

    // Find min / max extents
    if (screen_points[i][0] > maxs[0]) {
      maxs[0] = screen_points[i][0];
    }
    if (screen_points[i][1] > maxs[1]) {
      maxs[1] = screen_points[i][1];
    }

    if (screen_points[i][0] < mins[0]) {
      mins[0] = screen_points[i][0];
    }
    if (screen_points[i][1] < mins[1]) {
      mins[1] = screen_points[i][1];
    }

    // Find min / max projected depth to adjust far plane
    if (proj_point[1] > maxs[2]) {
      maxs[2] = proj_point[1];
    }
    if (proj_point[1] < mins[2]) {
      mins[2] = proj_point[1];
    }
  }
}

/**
 *
 */
CycleData *CascadeLight::CData::
make_copy() const {
  return new CData(*this);
}

/**
 *
 */
void CascadeLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *CascadeLight::
make_from_bam(const FactoryParams &params) {
  CascadeLight *cl = new CascadeLight("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  cl->fillin(scan, manager);
  return cl;
}
