/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamInt.I
 * @author brian
 * @date 2023-11-12
 */

/**
 *
 */
INLINE MaterialParamInt::
MaterialParamInt(const std::string &name, int default_value) :
  MaterialParamBase(name),
  _value(default_value)
{
}


/**
 *
 */
INLINE void MaterialParamInt::
set_value(int value) {
  _value = value;
}

/**
 *
 */
INLINE int MaterialParamInt::
get_value() const {
  return _value;
}
