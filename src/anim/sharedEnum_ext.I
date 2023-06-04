/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sharedEnum_ext.I
 * @author brian
 * @date 2021-06-14
 */

/**
 * Short-hand Python accessor to retrieve enum value IDs by name.
 */
INLINE PyObject *Extension<SharedEnum>::
__getattr__(PyObject *self, const std::string &attr_name) const {
  return Dtool_WrapValue(_this->get_value_id(attr_name));
}

/**
 * Sets all the enumerations as native Python attributes on the given
 * object.  This allows for quicker value lookup from Python code.
 */
INLINE void Extension<SharedEnum>::
fill_python_object(PyObject *obj) const {
  for (auto it = _this->_value_ids.begin(); it != _this->_value_ids.end(); ++it) {
    PyObject_SetAttrString(obj, it->first.c_str(), PyLong_FromLong((long)it->second));
  }
}
