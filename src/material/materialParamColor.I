/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamColor.I
 * @author brian
 * @date 2021-03-07
 */

/**
 *
 */
INLINE MaterialParamColor::
MaterialParamColor(const std::string &name, const LColor &default_value) :
  MaterialParamBase(name),
  _value(default_value)
{
}

/**
 *
 */
INLINE void MaterialParamColor::
set_value(const LColor &value) {
  _value = value;
}

/**
 *
 */
INLINE const LColor &MaterialParamColor::
get_value() const {
  return _value;
}
