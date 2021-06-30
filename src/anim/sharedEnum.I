/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sharedEnum.I
 * @author brian
 * @date 2021-06-14
 */

/**
 * Returns the ID of the value with the indicated name, or -1 if no such value
 * exists.
 */
INLINE int SharedEnum::
get_value_id(const std::string &name) const {
  auto it = _value_ids.find(name);
  if (it == _value_ids.end()) {
    return -1;
  }
  return (*it).second;
}

/**
 * Returns the name of the value with the indicated ID, or the empty string if
 * no value has the indicated ID.
 */
INLINE std::string SharedEnum::
get_value_name(int id) const {
  auto it = _value_names.find(id);
  if (it == _value_names.end()) {
    return "";
  }
  return (*it).second;
}
