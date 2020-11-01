/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file directionalLight.I
 * @author mike
 * @date 1999-02-04
 */

/**
 *
 */
INLINE DirectionalLight::CData::
CData() :
  _point(0.0f, 0.0f, 0.0f),
  _direction(LVector3::forward())
{
}

/**
 *
 */
INLINE DirectionalLight::CData::
CData(const DirectionalLight::CData &copy) :
  _point(copy._point),
  _direction(copy._direction)
{
}

/**
 * Returns the point in space at which the light is located.  This is local to
 * the coordinate space in which the light is assigned.
 *
 * This actually has no bearing on the visual effect of the light, since the
 * light is rendered as if it were infinitely far away.  This is only used to
 * create a visible representation of the light.
 */
INLINE const LPoint3 &DirectionalLight::
get_point() const {
  CDReader cdata(_cycler);
  return cdata->_point;
}

/**
 * Sets the point in space at which the light is located.
 */
INLINE void DirectionalLight::
set_point(const LPoint3 &point) {
  CDWriter cdata(_cycler);
  cdata->_point = point;
  mark_viz_stale();
}

/**
 * Returns the direction in which the light is aimed.  This is local to the
 * coordinate space in which the light is assigned.
 */
INLINE const LVector3 &DirectionalLight::
get_direction() const {
  CDReader cdata(_cycler);
  return cdata->_direction;
}

/**
 * Sets the direction in which the light is aimed.
 */
INLINE void DirectionalLight::
set_direction(const LVector3 &direction) {
  CDWriter cdata(_cycler);
  cdata->_direction = direction;
  mark_viz_stale();
}
