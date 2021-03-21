/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialBase.I
 * @author lachbr
 * @date 2021-03-06
 */

/**
 * Returns the material parameter with the indicated name, or NULL if no such
 * parameter exists.
 */
INLINE MaterialParamBase *MaterialBase::
get_param(CPT_InternalName name) const {
  Params::const_iterator pi = _params.find(name);
  if (pi == _params.end()) {
    return nullptr;
  }

  return (*pi).second;
}

/**
 * Sets the filename of the material.
 */
INLINE void MaterialBase::
set_filename(const Filename &filename) {
  _filename = filename;
}

/**
 *
 */
INLINE const Filename &MaterialBase::
get_filename() const {
  return _filename;
}

/**
 *
 */
INLINE void MaterialBase::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

/**
 *
 */
INLINE const Filename &MaterialBase::
get_fullpath() const {
  return _fullpath;
}

/**
 * Sets a new parameter on the material.  Replaces any existing parameter with
 * the same name.
 */
INLINE void MaterialBase::
set_param(MaterialParamBase *param) {
  _params[param->get_name()] = param;
}

/**
 * Removes the indicated parameter from the material.  Quietly does nothing if
 * the parameter was not actually part of this material.
 */
INLINE void MaterialBase::
clear_param(MaterialParamBase *param) {
  Params::iterator pi = _params.find(param->get_name());
  if (pi == _params.end()) {
    return;
  }

  _params.erase(pi);
}
