/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.I
 * @author lachbr
 * @date 2021-03-21
 */

/**
 *
 */
INLINE MaterialAttrib::
MaterialAttrib() :
  _material(nullptr),
  _has_raw_data(false),
  _is_off(false)
{
}

/**
 * Returns the material referenced by this material attribute.
 */
INLINE Material *MaterialAttrib::
get_material() const {
  return _material;
}

/**
 * Returns true if this is an "off" material attribute, false otherwise.
 */
INLINE bool MaterialAttrib::
is_off() const {
  return _is_off;
}
