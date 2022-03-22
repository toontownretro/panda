/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamMatrix.I
 * @author brian
 * @date 2022-03-22
 */

/**
 *
 */
INLINE MaterialParamMatrix::
MaterialParamMatrix(const std::string &name, const LMatrix4 &default_value) :
  MaterialParamBase(name),
  _value(default_value)
{
}

/**
 *
 */
INLINE void MaterialParamMatrix::
set_value(const LMatrix4 &value) {
  _value = value;
}

/**
 *
 */
INLINE const LMatrix4 &MaterialParamMatrix::
get_value() const {
  return _value;
}
