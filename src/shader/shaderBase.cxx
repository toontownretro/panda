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
#include "renderState.h"
#include "fogAttrib.h"
#include "clipPlaneAttrib.h"
#include "alphaTestAttrib.h"
#include "shaderAttrib.h"
#include "lightAttrib.h"
#include "lightRampAttrib.h"
#include "auxBitplaneAttrib.h"
#include "cascadeLight.h"
#include "transparencyAttrib.h"
#include "geomVertexAnimationSpec.h"
#include "postProcessDefines.h"
#include "depthWriteAttrib.h"
#include "lightMutexHolder.h"

TypeHandle ShaderBase::_type_handle;

/**
 * Registers a shader instance with the shader manager.
 */
void ShaderBase::
register_shader(ShaderBase *shader) {
  ShaderManager *mgr = ShaderManager::get_global_ptr();
  mgr->register_shader(shader);
}

/**
 *
 */
void ShaderBase::
register_shader(ShaderBase *shader, TypeHandle material_type) {
  ShaderManager *mgr = ShaderManager::get_global_ptr();
  mgr->register_shader(shader, material_type);
}

/**
 *
 */
void ShaderBase::
clear_cache() {
  LightMutexHolder holder(_lock);
  _cache.clear();
  _obj_cache.clear();
}
