/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_shader.cxx
 * @author brian
 * @date 2020-10-12
 */

#include "config_shader.h"
#include "shaderBase.h"
#include "shaderManager.h"
#include "shaderObject.h"
#include "depthShader.h"
#include "csmDepthShader.h"
//#include "vertexLitShader.h"
//#include "noMatShader.h"
#include "eyeRefractShader.h"
//#include "skyBoxShader.h"
#include "sourceShader.h"
#include "sourceSkyShader.h"
#include "sourceWater.h"
#include "sourceLightmappedShader.h"
//#include "lightmappedShader.h"
#include "twoTextureShader.h"
#include "spriteParticleShader.h"
#include "pbrShader.h"

#include "basicShader.h"

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

ConfigVariableEnum<ShaderManagerEnums::ShaderQuality> &
config_get_shader_quality() {
  static ConfigVariableEnum<ShaderManagerEnums::ShaderQuality> *shader_quality = nullptr;
  if (!shader_quality) {
    shader_quality = new ConfigVariableEnum<ShaderManagerEnums::ShaderQuality>
      ("shader-quality", ShaderManagerEnums::SQ_high,
      PRC_DESC("Sets the default quality level for all shaders.  This may not have "
               "any meaning to certain shaders.  It is up to the shader "
               "implementation to respect the chosen quality level."));
  }

  return *shader_quality;
}

ConfigVariableBool &
config_get_use_vertex_lit_for_no_material() {
  static ConfigVariableBool *use_vertex_lit_for_no_material = nullptr;
  if (use_vertex_lit_for_no_material == nullptr) {
    use_vertex_lit_for_no_material = new ConfigVariableBool
      ("use-vertex-lit-for-no-material", false,
       PRC_DESC("If true, uses the VertexLit shader for RenderStates with no "
                "material applied.  This allows for games that don't use Materials "
                "to still have lighting and shadows, albeit with almost no "
                "configurability.  When this is false, RenderStates without "
                "Materials use the NoMat shader, which renders a single unlit "
                "texture."));
  }

  return *use_vertex_lit_for_no_material;
}

ConfigVariableFilename default_cube_map
("default-cube-map", Filename(),
  PRC_DESC("Specifies the default cube map texture to use for a material "
          "that requests an environmental cube map but there are no nearby "
          "cube maps."));

ConfigVariableInt shadow_offset_window_size("shadow-offset-window-size", 16);
ConfigVariableInt shadow_offset_filter_size("shadow-offset-filter-size", 8);
ConfigVariableDouble shadow_pcss_light_size("shadow-pcss-light-size", 1.0);
ConfigVariableDouble shadow_pcss_softness("shadow-pcss-softness", 1.0);

ConfigVariableBool shader_dynamic_compilation
("shader-dynamic-compilation", false,
 PRC_DESC("If true, shaders will be read from source and variations compiled on the fly"
          "when requested.  This helps save development time when working on shaders, as "
          "you no longer have to recompile the all variations offline if you modify a "
          "shader.  Note that enabling this will cause massive hitches the first time "
          "a variation is requested."));

ConfigVariableBool cubemaps_enabled
("cubemaps-enabled", true,
 PRC_DESC("Enable/disable cubemap reflections."));

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
  //NoMatShader::init_type();
  //VertexLitShader::init_type();
  EyeRefractShader::init_type();
  //LightmappedShader::init_type();
  //SkyBoxShader::init_type();
  SourceShader::init_type();
  SourceSkyShader::init_type();
  SourceWater::init_type();
  SourceLightmappedShader::init_type();
  TwoTextureShader::init_type();
  SpriteParticleShader::init_type();
  PBRShader::init_type();

  BasicShader::init_type();

  ShaderObject::init_type();
  ShaderObject::register_with_read_factory();
}
