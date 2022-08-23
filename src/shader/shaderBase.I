/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.I
 * @author lachbr
 * @date 2020-10-16
 */

/**
 *
 */
INLINE ShaderBase::
ShaderBase(const std::string &name) :
  Namable(downcase(name)),
  _lock("lock") {
}


/**
 * Adds an alternative name that can be used to refer to this shader.
 */
INLINE void ShaderBase::
add_alias(const std::string &name) {
  _aliases.push_back(downcase(name));
}

/**
 * Returns the number of alternative names for this shader.
 */
INLINE size_t ShaderBase::
get_num_aliases() const {
  return _aliases.size();
}

/**
 * Returns the nth alternative name for this shader.
 */
INLINE const std::string &ShaderBase::
get_alias(size_t n) const {
  return _aliases[n];
}
