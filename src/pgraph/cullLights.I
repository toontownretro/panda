/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullLights.I
 * @author brian
 * @date 2020-12-10
 */

/**
 *
 */
INLINE CullLights::
CullLights() {
}

/**
 *
 */
INLINE CullLights::
CullLights(const CullLights &copy) :
  _lights(copy._lights)
{
}

/**
 *
 */
INLINE void CullLights::
operator = (const CullLights &copy) {
  _lights = copy._lights;
}

/**
 *
 */
INLINE CullLights::
~CullLights() {
}

/**
 * Returns true if there are no lights listed in the CullLights object.
 */
INLINE bool CullLights::
is_empty() const {
  return _lights.empty();
}
