/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.I
 * @author lachbr
 * @date 2020-10-16
 */

/**
 * Returns the shader instance by name, or nullptr if no shader exists with
 * that name.
 */
INLINE ShaderBase *ShaderManager::
get_shader(const std::string &name) const {
  ShaderRegistry::const_iterator it = _shaders.find(name);
  if (it != _shaders.end()) {
    return (*it).second;
  }

  return nullptr;
}
