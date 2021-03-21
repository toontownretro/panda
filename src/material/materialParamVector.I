/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamVector.I
 * @author lachbr
 * @date 2021-03-07
 */

/**
 *
 */
INLINE MaterialParamVector::
MaterialParamVector(const std::string &name, const LVector3 &default_value) :
  MaterialParamBase(name),
  _value(default_value)
{
}


/**
 *
 */
INLINE void MaterialParamVector::
set_value(const LVector3 &value) {
  _value = value;
}

/**
 *
 */
INLINE const LVector3 &MaterialParamVector::
get_value() const {
  return _value;
}
