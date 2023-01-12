/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamFloat.I
 * @author brian
 * @date 2021-03-07
 */

/**
 *
 */
INLINE MaterialParamFloat::
MaterialParamFloat(const std::string &name, PN_stdfloat default_value) :
  MaterialParamBase(name),
  _value(default_value)
{
}


/**
 *
 */
INLINE void MaterialParamFloat::
set_value(PN_stdfloat value) {
  _value = value;
}

/**
 *
 */
INLINE PN_stdfloat MaterialParamFloat::
get_value() const {
  return _value;
}
