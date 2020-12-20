/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointLight.I
 * @author mike
 * @date 1999-02-04
 */

/**
 *
 */
INLINE PointLight::CData::
CData() :
  _falloff(1.0),
  _inner_radius(1.0),
  _outer_radius(2.0),
  _point(0.0f, 0.0f, 0.0f)
{
}

/**
 *
 */
INLINE PointLight::CData::
CData(const PointLight::CData &copy) :
  _falloff(copy._falloff),
  _inner_radius(copy._inner_radius),
  _outer_radius(copy._outer_radius),
  _point(copy._point)
{
}

/**
 * Returns the inverse-square falloff coefficient for the light.
 */
INLINE PN_stdfloat PointLight::
get_falloff() const {
  CDReader cdata(_cycler);
  return cdata->_falloff;
}

/**
 * Sets the inverse-square falloff coefficient for the light.
 */
INLINE void PointLight::
set_falloff(PN_stdfloat falloff) {
  CDWriter cdata(_cycler);
  cdata->_falloff = falloff;
}

/**
 * Returns the inner radius of the light.  This is the area of the light that
 * attenuates according to the inverse-square falloff coefficient.
 */
INLINE PN_stdfloat PointLight::
get_inner_radius() const {
  CDReader cdata(_cycler);
  return cdata->_inner_radius;
}

/**
 * Sets the inner radius of the light.  This is the area of the light that
 * attenuates according to the inverse-square falloff coefficient.
 */
INLINE void PointLight::
set_inner_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_inner_radius = radius;
}

/**
 * Returns the outer radius of the light.  This is the area of the light that
 * linearly attenuates to 0 at the edge of the sphere.
 */
INLINE PN_stdfloat PointLight::
get_outer_radius() const {
  CDReader cdata(_cycler);
  return cdata->_outer_radius;
}

/**
 * Sets the outer radius of the light.  This is the area of the light that
 * linearly attenuates to 0 at the edge of the sphere.
 */
INLINE void PointLight::
set_outer_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_outer_radius = radius;
  for (int i = 0; i < get_num_lenses(); i++) {
    get_lens(i)->set_far(radius);
  }
}

/**
 * Returns the point in space at which the light is located.  This is local to
 * the coordinate space in which the light is assigned, and is usually 0.
 */
INLINE const LPoint3 &PointLight::
get_point() const {
  CDReader cdata(_cycler);
  return cdata->_point;
}

/**
 * Sets the point in space at which the light is located.  Usually 0.
 */
INLINE void PointLight::
set_point(const LPoint3 &point) {
  CDWriter cdata(_cycler);
  cdata->_point = point;
  mark_viz_stale();
}
