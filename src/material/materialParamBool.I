/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamBool.I
 * @author lachbr
 * @date 2021-03-07
 */

/**
 *
 */
INLINE MaterialParamBool::
MaterialParamBool(const std::string &name, bool default_value) :
  MaterialParamBase(name),
  _value(default_value)
{
}

/**
 *
 */
INLINE void MaterialParamBool::
set_value(bool value) {
  _value = value;
}

/**
 *
 */
INLINE bool MaterialParamBool::
get_value() const {
  return _value;
}
