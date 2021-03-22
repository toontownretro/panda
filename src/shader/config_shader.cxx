/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_shader.cxx
 * @author lachbr
 * @date 2020-10-12
 */

#include "config_shader.h"
#include "shaderBase.h"
#include "shaderManager.h"
#include "shaderObject.h"
#include "depthShader.h"
#include "csmDepthShader.h"
#include "vertexLitShader.h"
#include "noMatShader.h"
//#include "lightmappedShader.h"

ConfigureDef(config_shader);
ConfigureFn(config_shader) {
  init_libshader();
}

NotifyCategoryDef(shadermgr, "");

ConfigVariableList &
get_shader_libraries() {
  static ConfigVariableList *load_shader_library = nullptr;
  if (!load_shader_library) {
    load_shader_library = new ConfigVariableList
      ("load-shader-library",
       PRC_DESC("Specifies the shader libraries to load."));
  }

  return *load_shader_library;
}

ConfigVariableEnum<ShaderEnums::ShaderQuality> &
config_get_shader_quality() {
  static ConfigVariableEnum<ShaderEnums::ShaderQuality> *shader_quality = nullptr;
  if (!shader_quality) {
    shader_quality = new ConfigVariableEnum<ShaderEnums::ShaderQuality>
      ("shader-quality", ShaderEnums::SQ_high,
      PRC_DESC("Sets the default quality level for all shaders.  This may not have "
                "any meaning to certain shaders.  It is up to the shader "
                "implementation to respect the chosen quality level."));
  }

  return *shader_quality;
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libshader() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  ShaderBase::init_type();
  ShaderManager::get_global_ptr()->load_shader_libraries();

  DepthShader::init_type();
  CSMDepthShader::init_type();
  VertexLitShader::init_type();
  NoMatShader::init_type();
  //LightmappedShader::init_type();

  ShaderObject::init_type();
  ShaderObject::register_with_read_factory();
}
