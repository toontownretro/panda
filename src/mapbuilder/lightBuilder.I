/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightBuilder.I
 * @author brian
 * @date 2021-09-18
 */

/**
 * Sets a constant bias value to apply to all rays casted into the scene.
 * This is used to solve acne problems.
 */
INLINE void LightBuilder::
set_ray_bias(PN_stdfloat bias) {
  _bias = bias;
}

/**
 * Returns a constant bias value to apply to all rays casted into the scene.
 */
INLINE PN_stdfloat LightBuilder::
get_ray_bias() const {
  return _bias;
}

/**
 * Sets the number of bounces to perform during the indirect lighting pass.
 * More bounces results in more accurate indirect lighting at the cost of
 * increased computation time.
 */
INLINE void LightBuilder::
set_num_bounces(int bounces) {
  _bounces = bounces;
}

/**
 * Returns the number of bounces to perform during the indirect lighting pass.
 */
INLINE int LightBuilder::
get_num_bounces() const {
  return _bounces;
}

/**
 * Sets the total number of rays to cast from each luxel during each bounce
 * of the indirect lighting pass.
 */
INLINE void LightBuilder::
set_num_rays_per_luxel(int count) {
  _rays_per_luxel = count;
}

/**
 * Returns the number of rays to cast from each luxel during each bounce of
 * the indirect lighting pass.
 */
INLINE int LightBuilder::
get_num_rays_per_luxel() const {
  return _rays_per_luxel;
}

/**
 *
 */
INLINE void LightBuilder::
set_ray_region_size(int size) {
  _ray_region_size = size;
}

/**
 *
 */
INLINE int LightBuilder::
get_ray_region_size() const {
  return _ray_region_size;
}

/**
 *
 */
INLINE void LightBuilder::
set_num_rays_per_region(int count) {
  _rays_per_region = count;
}

/**
 *
 */
INLINE int LightBuilder::
get_num_rays_per_region() const {
  return _rays_per_region;
}

/**
 * Sets the color of the sky/ambient light.  This color will be used for
 * bounce rays that hit nothing.
 */
INLINE void LightBuilder::
set_sky_color(const LColor &color) {
  _sky_color = color;
}

/**
 * Returns the color of the sky/ambient light.
 */
INLINE LColor LightBuilder::
get_sky_color() const {
  return _sky_color;
}

/**
 *
 */
INLINE void LightBuilder::
set_sun_angular_extent(PN_stdfloat angle) {
  _sun_angular_extent = angle;
}

/**
 *
 */
INLINE PN_stdfloat LightBuilder::
get_sun_angular_extent() const {
  return _sun_angular_extent;
}
