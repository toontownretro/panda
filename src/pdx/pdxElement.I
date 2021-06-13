/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxElement.I
 * @author brian
 * @date 2021-06-10
 */

/**
 *
 */
INLINE void PDXElement::
set_attribute(const std::string &name, const PDXValue &value) {
  _attribs[name] = value;
}

/**
 *
 */
INLINE int PDXElement::
get_num_attributes() const {
  return (int)_attribs.size();
}

/**
 *
 */
INLINE const std::string &PDXElement::
get_attribute_name(int n) const {
  return _attribs.get_key(n);
}

/**
 *
 */
INLINE const PDXValue &PDXElement::
get_attribute_value(int n) const {
  return _attribs.get_data(n);
}

/**
 *
 */
INLINE PDXValue &PDXElement::
get_attribute_value(int n) {
  return _attribs.modify_data(n);
}

/**
 *
 */
INLINE PDXValue &PDXElement::
get_attribute_value(const std::string &name) {
  return _attribs[name];
}

/**
 *
 */
INLINE int PDXElement::
find_attribute(const std::string &name) const {
  return _attribs.find(name);
}

/**
 *
 */
INLINE bool PDXElement::
has_attribute(const std::string &name) const {
  return find_attribute(name) != -1;
}

/**
 *
 */
INLINE void PDXElement::
remove_attribute(const std::string &name) {
  _attribs.remove(name);
}

/**
 *
 */
INLINE void PDXElement::
remove_attribute(int n) {
  _attribs.remove_element(n);
}

/**
 *
 */
INLINE PDXValue &PDXElement::
operator [] (const std::string &name) {
  return get_attribute_value(name);
}
