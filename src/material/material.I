/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.I
 * @author lachbr
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
