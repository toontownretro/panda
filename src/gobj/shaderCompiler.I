/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderCompiler.I
 * @author brian
 * @date 2020-12-23
 */

/**
 *
 */
INLINE ShaderCompiler::Options::Define::
Define() {
  name = nullptr;
  value = 0;
}

/**
 *
 */
INLINE ShaderCompiler::Options::Define::
Define(const Define &other) :
  name(other.name),
  value(other.value) {
}

/**
 * Sets a define that should be set when the shader is compiled.
 */
INLINE void ShaderCompiler::Options::
set_define(CPT_InternalName name, int value) {
  auto it = std::find(_defines.begin(), _defines.end(), name);
  if (it == _defines.end()) {
    Define define;
    define.name = name;
    define.value = value;
    _defines.push_back(define);

  } else {
    (*it).value = value;
  }
}

/**
 * Returns the number of defines.
 */
INLINE size_t ShaderCompiler::Options::
get_num_defines() const {
  return _defines.size();
}

/**
 * Returns the define at the indicated index.
 */
INLINE const ShaderCompiler::Options::Define *ShaderCompiler::Options::
get_define(size_t n) const {
  return &_defines[n];
}

/**
 * Returns the define with the indicated name.
 */
INLINE const ShaderCompiler::Options::Define *ShaderCompiler::Options::
get_define(CPT_InternalName name) const {
  auto it = std::find(_defines.begin(), _defines.end(), name);
  if (it == _defines.end()) {
    return nullptr;
  }

  return &(*it);
}
