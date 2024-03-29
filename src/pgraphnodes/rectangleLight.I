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
  _max_distance(make_inf((PN_stdfloat)0))
{
}

/**
 *
 */
INLINE RectangleLight::CData::
CData(const RectangleLight::CData &copy) :
  _max_distance(copy._max_distance)
{
}

/**
 * Returns the maximum distance at which the light has any effect, as previously
 * specified by set_max_distance.
 */
INLINE PN_stdfloat RectangleLight::
get_max_distance() const {
  CDReader cdata(_cycler);
  return cdata->_max_distance;
}

/**
 * Sets the radius of the light's sphere of influence.  Beyond this distance, the
 * light may be attenuated to zero, if this is supported by the shader.
 */
INLINE void RectangleLight::
set_max_distance(PN_stdfloat max_distance) {
  CDWriter cdata(_cycler);
  cdata->_max_distance = max_distance;
}
