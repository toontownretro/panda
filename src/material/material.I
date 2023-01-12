/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.I
 * @author brian
 * @date 2021-03-06
 */

/**
 * Returns the number of parameters in the material.
 */
INLINE size_t Material::
get_num_params() const {
  return _params.size();
}

/**
 * Returns the nth parameter in the material.
 */
INLINE MaterialParamBase *Material::
get_param(size_t n) const {
  return _params.get_data(n);
}

/**
 * Returns the material parameter with the indicated name, or NULL if no such
 * parameter exists.
 */
INLINE MaterialParamBase *Material::
get_param(CPT_InternalName name) const {
  int index = _params.find(name);
  if (index == -1) {
    return nullptr;
  }

  return _params.get_data(index);
}

/**
 * Sets the filename of the material.
 */
INLINE void Material::
set_filename(const Filename &filename) {
  _filename = filename;
}

/**
 *
 */
INLINE const Filename &Material::
get_filename() const {
  return _filename;
}

/**
 *
 */
INLINE void Material::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

/**
 *
 */
INLINE const Filename &Material::
get_fullpath() const {
  return _fullpath;
}

/**
 * Adds a tag to the material.
 */
INLINE void Material::
set_tag(const std::string &tag, std::string &value) {
  _tags[tag] = value;
}

/**
 * Removes the indicated tag from the material.
 */
INLINE void Material::
clear_tag(const std::string &tag) {
  _tags.remove(tag);
}

/**
 * Removes the tag with the indicated index from the material.
 */
INLINE void Material::
clear_tag(int n) {
  _tags.remove_element(n);
}

/**
 * Returns true if the material has the indicated tag, false otherwise.
 */
INLINE bool Material::
has_tag(const std::string &tag) const {
  return _tags.find(tag) != -1;
}

/**
 * Returns the number of tags on the material.
 */
INLINE int Material::
get_num_tags() const {
  return (int)_tags.size();
}

/**
 * Returns the tag value associated with the indicated key.
 */
INLINE std::string Material::
get_tag_value(const std::string &key) const {
  int it = _tags.find(key);
  if (it == -1) {
    return std::string();
  }
  return _tags.get_data(it);
}

/**
 * Returns the material's nth tag key.
 */
INLINE std::string Material::
get_tag_key(int n) const {
  return _tags.get_key(n);
}

/**
 * Returns the material's nth tag key.
 */
INLINE std::string Material::
get_tag_value(int n) const {
  return _tags.get_data(n);
}

/**
 * Sets a new parameter on the material.  Replaces any existing parameter with
 * the same name.
 */
INLINE void Material::
set_param(MaterialParamBase *param) {
  _params[param->get_name()] = param;
}

/**
 * Removes the indicated parameter from the material.  Quietly does nothing if
 * the parameter was not actually part of this material.
 */
INLINE void Material::
clear_param(MaterialParamBase *param) {
  _params.remove(param->get_name());
}
