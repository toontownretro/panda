/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spotlight.I
 * @author mike
 * @date 1999-02-04
 */

/**
 *
 */
INLINE Spotlight::CData::
CData() :
  _exponent(1.0f),
  _attenuation(0.0f, 0.0f, 1.0f),
  _max_distance(make_inf((PN_stdfloat)0)),
  _inner_cone(35.0f)
{
}

/**
 *
 */
INLINE Spotlight::CData::
CData(const Spotlight::CData &copy) :
  _exponent(copy._exponent),
  _attenuation(copy._attenuation),
  _max_distance(copy._max_distance),
  _inner_cone(copy._inner_cone)
{
}

/**
 * Returns the exponent that controls the amount of light falloff from the
 * center of the spotlight.  See set_exponent().
 */
INLINE PN_stdfloat Spotlight::
get_exponent(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_exponent;
}

/**
 * Sets the exponent that controls the amount of light falloff from the center
 * of the spotlight.  The light is attenuated by the cosine of the angle
 * between the direction of the light and the direction of the point being
 * lighted, raised to the power of this exponent.  Thus, higher exponents
 * result in a more focused light source, regardless of the field-of-view of
 * the lens.
 */
INLINE void Spotlight::
set_exponent(PN_stdfloat exponent) {
  CDWriter cdata(_cycler);
  cdata->_exponent = exponent;
}

/**
 * Returns the terms of the attenuation equation for the light.  These are, in
 * order, the constant, linear, and quadratic terms based on the distance from
 * the point to the vertex.
 */
INLINE const LVecBase3 &Spotlight::
get_attenuation(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_attenuation;
}

/**
 * Sets the terms of the attenuation equation for the light.  These are, in
 * order, the constant, linear, and quadratic terms based on the distance from
 * the point to the vertex.
 */
INLINE void Spotlight::
set_attenuation(const LVecBase3 &attenuation) {
  CDWriter cdata(_cycler);
  cdata->_attenuation = attenuation;
}

/**
 * Returns the maximum distance at which the light has any effect, as previously
 * specified by set_max_distance.
 */
INLINE PN_stdfloat Spotlight::
get_max_distance() const {
  CDReader cdata(_cycler);
  return cdata->_max_distance;
}

/**
 * Sets the radius of the light's sphere of influence.  Beyond this distance, the
 * light may be attenuated to zero, if this is supported by the shader.
 */
INLINE void Spotlight::
set_max_distance(PN_stdfloat max_distance) {
  CDWriter cdata(_cycler);
  cdata->_max_distance = max_distance;
  get_lens()->set_far(max_distance);
}

/**
 * Returns the angle of the outer cone of the spotlight.  The outer cone has
 * angular falloff.
 */
INLINE PN_stdfloat Spotlight::
get_outer_cone(Thread *current_thread) const {
  // The outer angle is stored as the field of view of the lens associated
  // with the light.
  return get_lens()->get_fov(current_thread)[0];
}

/**
 * Sets the angle of the outer cone of the spotlight.  The outer cone has
 * angular falloff.
 */
INLINE void Spotlight::
set_outer_cone(PN_stdfloat angle) {
  // The outer angle is stored as the field of view of the lens associated
  // with the light.
  get_lens()->set_fov(angle);
}

/**
 * Returns the angle of the inner cone of the spotlight.  The inner cone does
 * not have angular falloff.
 */
INLINE PN_stdfloat Spotlight::
get_inner_cone(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_inner_cone;
}

/**
 * Sets the angle of the inner cone of the spotlight.  The inner cone does not
 * have angular falloff.
 */
INLINE void Spotlight::
set_inner_cone(PN_stdfloat angle) {
  CDWriter cdata(_cycler);
  cdata->_inner_cone = angle;
}
