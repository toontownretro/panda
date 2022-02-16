/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cascadeLight.I
 * @author lachbr
 * @date 2020-10-29
 */

/**
 * Returns the main scene camera that the cascades are calculated against.
 */
INLINE NodePath CascadeLight::
get_scene_camera() const {
  return _scene_camera.get_node_path();
}

/**
 * Sets the main scene camera that the cascades are calculated against.
 */
INLINE void CascadeLight::
set_scene_camera(const NodePath &camera) {
  _scene_camera = camera;
  // We need to use the scene camera as the reference point for computing LOD
  // distances when rendering to the shadow maps.  By default, the light node
  // itself is used as the reference point.  This would cause incorrect LODs
  // because it is the cascades themselves that follow the scene camera, the
  // light remains stationary.
  set_lod_center(camera);
}

/**
 * Returns the number of cascades the light is using.
 */
INLINE int CascadeLight::
get_num_cascades() const {
  return _num_cascades;
}

/**
 * Sets the number of cascades the light should render shadow maps into.
 */
INLINE void CascadeLight::
set_num_cascades(int num) {
  _num_cascades = num;
  if (_shadow_caster) {
    // Recreate stuff
    clear_shadow_buffers();
    setup_shadow_map();
  }
}

/**
 * Returns the maximum distance that cascaded shadows will be rendered.
 */
INLINE float CascadeLight::
get_csm_distance() const {
  return _csm_distance;
}

/**
 * Sets the maximum distance that cascaded shadows will be rendered.
 */
INLINE void CascadeLight::
set_csm_distance(float distance) {
  _csm_distance = distance;
}

/**
 * Returns the distance that the "sun" is from the camera.  Really, this is
 * how far from the scene camera the near plane of the cascades are.  A larger
 * sun distance will get more objects in shadow.
 */
INLINE float CascadeLight::
get_sun_distance() const {
  return _sun_distance;
}

/**
 * Sets the distance that the "sun" is from the camera.  Really, this is
 * how far from the scene camera the near plane of the cascades are.  A larger
 * sun distance will get more objects in shadow.
 */
INLINE void CascadeLight::
set_sun_distance(float distance) {
  _sun_distance = distance;
}

/**
 * Returns whether or not we should use a fixed film size for the cascades.  This
 * can help reduce jittery-ness because the film size of the cascades are not
 * being adjusted all the time.
 */
INLINE bool CascadeLight::
get_use_fixed_film_size() const {
  return _fixed_film_size;
}

/**
 * Sets whether or not we should use a fixed film size for the cascades.  This
 * can help reduce jittery-ness because the film size of the cascades are not
 * being adjusted all the time.
 */
INLINE void CascadeLight::
set_use_fixed_film_size(bool flag) {
  _fixed_film_size = flag;
}

/**
 * Returns the logarithmic factor used to determine where to split the view
 * frustum to create the cascades.  A larger factor results in a more steep
 * splitting of the view frustum, where a smaller factor results in a more
 * linear split.
 */
INLINE float CascadeLight::
get_log_factor() const {
  return _log_factor;
}

/**
 * Sets the logarithmic factor used to determine where to split the view
 * frustum to create the cascades.  A larger factor results in a more steep
 * splitting of the view frustum, where a smaller factor results in a more
 * linear split.
 */
INLINE void CascadeLight::
set_log_factor(float factor) {
  _log_factor = factor;
}

/**
 * Returns the bias applied to the borders of each cascade.  This can help
 * reduce seams between cascades.
 */
INLINE float CascadeLight::
get_border_bias() const {
  return _border_bias;
}

/**
 * Sets the bias applied to the borders of each cascade.  This can help
 * reduce seams between cascades.
 */
INLINE void CascadeLight::
set_border_bias(float bias) {
  _border_bias = bias;
}

/**
 * Returns the MVP matrix for the indicated cascade.
 */
INLINE const LMatrix4 &CascadeLight::
get_cascade_mvp(int n) const {
  CDReader cdata(_cycler);
  return cdata->_cascade_mvps[n];
}

/**
 * Returns the near-far points for the indicated cascade.
 */
INLINE const LVecBase2 &CascadeLight::
get_cascade_near_far(int n) const {
  CDReader cdata(_cycler);
  return cdata->_cascade_nearfar[n];
}

/**
 *
 */
INLINE void CascadeLight::
get_cascade_atlas_mins_maxs(int n, LVecBase2 &mins, LVecBase2 &maxs) const {
  mins = _cascades[n].atlas_mins;
  maxs = _cascades[n].atlas_maxs;
}

/**
 *
 */
INLINE void CascadeLight::
get_cascade_atlas_scale(int n, LVecBase2 &scale) const {
  scale = _cascades[n].atlas_scale;
}

/**
 * Returns a value between 0 and 1 that is the point at which a particular
 * split should begin.
 */
INLINE float CascadeLight::
get_split_start(int n) const {
  float x = (float)n / _num_cascades;
  return ((std::exp(_log_factor * x) - 1) / (std::exp(_log_factor) - 1));
}

/**
 * Returns an interpolated point along a particular corner of the view frustum.
 */
INLINE LPoint3 CascadeLight::
get_interpolated_point(CascadeLight::CoordinateOrigin origin, float depth) {
  nassertr(depth >= 0.0f && depth <= 1.0f, LPoint3());
  return _curr_near_points[origin] * (1.0f - depth) + _curr_far_points[origin] * depth;
}

/**
 * Returns the MVP matrix for a given cascade.
 */
INLINE void CascadeLight::
compute_mvp(int n, LMatrix4 &mvp, const TransformState *to_local) {
  const Cascade &c = _cascades[n];
  mvp = to_local->get_mat() * c.lens->get_projection_mat();
}

/**
 * This takes a minimum and maximum extent in NDC space and computes
 * the film size and film offset needed to cover that extent.
 */
INLINE void CascadeLight::
get_film_properties(LVecBase2 &size, LVecBase2 &offset,
                    const LVecBase3 &mins, const LVecBase3 &maxs) {
  float x_center = (mins.get_x() + maxs.get_x()) * 0.5;
  float y_center = (mins.get_y() + maxs.get_y()) * 0.5;
  float x_size = maxs.get_x() - x_center;
  float y_size = maxs.get_y() - y_center;
  size.set(x_size, y_size);
  offset.set(x_center * 0.5, y_center * 0.5);
}

/**
 *
 */
INLINE CascadeLight::CData::
CData() :
  CycleData() {
}

/**
 *
 */
INLINE CascadeLight::CData::
CData(const CData &copy) :
  _cascade_mvps(copy._cascade_mvps),
  _cascade_nearfar(copy._cascade_nearfar) {
}
