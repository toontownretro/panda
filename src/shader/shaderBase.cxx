/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.cxx
 * @author lachbr
 * @date 2020-10-16
 */

#include "shaderBase.h"
#include "shaderManager.h"

TypeHandle ShaderBase::_type_handle;

/**
 * Resets the shader to a clean slate, ready for the state that needs a shader.
 */
void ShaderBase::
reset() {
  _language = Shader::SL_none;
  _flags = 0;
  _inputs.clear();
  _stage_flags = SF_none;
  for (int i = 0; i < S_COUNT; i++) {
    _stages[i].reset();
  }
}

/**
 * Registers a shader instance with the shader manager.
 */
void ShaderBase::
register_shader(ShaderBase *shader) {
  ShaderManager *mgr = ShaderManager::get_global_ptr();
  mgr->register_shader(shader);
}
