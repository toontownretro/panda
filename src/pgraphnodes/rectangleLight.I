/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rectangleLight.I
 * @author rdb
 * @date 2016-12-19
 */

/**
 *
 */
INLINE RectangleLight::CData::
CData() :
  _falloff(1.0),
  _inner_radius(1.0),
  _outer_radius(2.0)
{
}

/**
 *
 */
INLINE RectangleLight::CData::
CData(const RectangleLight::CData &copy) :
  _falloff(copy._falloff),
  _inner_radius(copy._inner_radius),
  _outer_radius(copy._outer_radius)
{
}

/**
 * Returns the inverse-square falloff coefficient for the light.
 */
INLINE PN_stdfloat RectangleLight::
get_falloff() const {
  CDReader cdata(_cycler);
  return cdata->_falloff;
}

/**
 * Sets the inverse-square falloff coefficient for the light.
 */
INLINE void RectangleLight::
set_falloff(PN_stdfloat falloff) {
  CDWriter cdata(_cycler);
  cdata->_falloff = falloff;
}

/**
 * Returns the inner radius of the light.  This is the area of the light that
 * attenuates according to the inverse-square falloff coefficient.
 */
INLINE PN_stdfloat RectangleLight::
get_inner_radius() const {
  CDReader cdata(_cycler);
  return cdata->_inner_radius;
}

/**
 * Sets the inner radius of the light.  This is the area of the light that
 * attenuates according to the inverse-square falloff coefficient.
 */
INLINE void RectangleLight::
set_inner_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_inner_radius = radius;
}

/**
 * Returns the outer radius of the light.  This is the area of the light that
 * linearly attenuates to 0 at the edge of the sphere.
 */
INLINE PN_stdfloat RectangleLight::
get_outer_radius() const {
  CDReader cdata(_cycler);
  return cdata->_outer_radius;
}

/**
 * Sets the outer radius of the light.  This is the area of the light that
 * linearly attenuates to 0 at the edge of the sphere.
 */
INLINE void RectangleLight::
set_outer_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_outer_radius = radius;
  get_lens()->set_far(radius);
}
