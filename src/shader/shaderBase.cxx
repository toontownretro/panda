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
#include "colorBlendAttrib.h"

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

/**
 * Returns true if the given RenderState enables additive framebuffer
 * blending.  If fogging is enabled, the fog color should be overridden
 * to black.
 */
bool ShaderBase::
has_additive_blend(const RenderState *state) const {
  const ColorBlendAttrib *cba;
  state->get_attrib_def(cba);

  return (cba->get_mode() == ColorBlendAttrib::M_add) &&
    (cba->get_operand_a() == ColorBlendAttrib::O_one) &&
    (cba->get_operand_b() == ColorBlendAttrib::O_one);
}

/**
 * Returns true if the given RenderState enables modulate framebuffer
 * blending.  In this mode, new pixels are multiplied with existing
 * pixels in the framebuffer.  If fogging is enabled, the fog color
 * should be overridden to gray.
 */
bool ShaderBase::
has_modulate_blend(const RenderState *state) const {
  const ColorBlendAttrib *cba;
  state->get_attrib_def(cba);

  return (cba->get_mode() == ColorBlendAttrib::M_add) &&
    (cba->get_operand_a() == ColorBlendAttrib::O_fbuffer_alpha) &&
    (cba->get_operand_b() == ColorBlendAttrib::O_incoming_color);
}
