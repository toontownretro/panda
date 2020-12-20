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
  _falloff(1.0f),
  _inner_radius(1.0),
  _outer_radius(2.0),
  _inner_cone(35.0f)
{
}

/**
 *
 */
INLINE Spotlight::CData::
CData(const Spotlight::CData &copy) :
  _exponent(copy._exponent),
  _falloff(copy._falloff),
  _inner_radius(copy._inner_radius),
  _outer_radius(copy._outer_radius),
  _inner_cone(copy._inner_cone)
{
}

/**
 * Returns the exponent that controls the amount of light falloff from the
 * center of the spotlight.  See set_exponent().
 */
INLINE PN_stdfloat Spotlight::
get_exponent() const {
  CDReader cdata(_cycler);
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
 * Returns the inverse-square falloff coefficient for the light.
 */
INLINE PN_stdfloat Spotlight::
get_falloff() const {
  CDReader cdata(_cycler);
  return cdata->_falloff;
}

/**
 * Sets the inverse-square falloff coefficient for the light.
 */
INLINE void Spotlight::
set_falloff(PN_stdfloat falloff) {
  CDWriter cdata(_cycler);
  cdata->_falloff = falloff;
}

/**
 * Returns the inner radius of the light.  This is the area of the light that
 * attenuates according to the inverse-square falloff coefficient.
 */
INLINE PN_stdfloat Spotlight::
get_inner_radius() const {
  CDReader cdata(_cycler);
  return cdata->_inner_radius;
}

/**
 * Sets the inner radius of the light.  This is the area of the light that
 * attenuates according to the inverse-square falloff coefficient.
 */
INLINE void Spotlight::
set_inner_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_inner_radius = radius;
}

/**
 * Returns the outer radius of the light.  This is the area of the light that
 * linearly attenuates to 0 at the edge of the sphere.
 */
INLINE PN_stdfloat Spotlight::
get_outer_radius() const {
  CDReader cdata(_cycler);
  return cdata->_outer_radius;
}

/**
 * Sets the outer radius of the light.  This is the area of the light that
 * linearly attenuates to 0 at the edge of the sphere.
 */
INLINE void Spotlight::
set_outer_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_outer_radius = radius;
  get_lens()->set_far(radius);
}

/**
 * Returns the angle of the outer cone of the spotlight.  The outer cone has
 * angular falloff.
 */
INLINE PN_stdfloat Spotlight::
get_outer_cone() const {
  // The outer angle is stored as the field of view of the lens associated
  // with the light.
  return get_lens()->get_fov()[0];
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
get_inner_cone() const {
  CDReader cdata(_cycler);
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
