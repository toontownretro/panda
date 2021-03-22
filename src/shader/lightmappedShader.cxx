/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightmappedShader.cxx
 * @author lachbr
 * @date 2021-01-02
 */

#if 0

#include "lightmappedShader.h"
#include "renderState.h"
#include "textureStage.h"
#include "texture.h"
#include "textureAttrib.h"

TypeHandle LightmappedShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void LightmappedShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const ParamAttrib *params,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/lightmappedGeneric_PBR.vert.glsl");
  set_pixel_shader("shaders/lightmappedGeneric_PBR.frag.glsl");

  add_shader_quality();
  add_transparency(state);
  add_alpha_test(state);
  add_hdr(state);

  add_aux_attachments(state);

  // Find the textures in use.
  const TextureAttrib *ta;
  state->get_attrib_def(ta);
  int num_stages = ta->get_num_on_stages();
  for (int i = 0; i < num_stages; i++) {
    TextureStage *stage = ta->get_on_stage(i);
    const std::string &stage_name = stage->get_name();

    if (stage == TextureStage::get_default() ||
        stage_name == "albedo") {
      set_pixel_shader_define("BASETEXTURE");
      set_input(ShaderInput("baseTextureSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "arme") {
      set_pixel_shader_define("ARME");
      set_input(ShaderInput("armeSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "reflection") {
      set_pixel_shader_define("PLANAR_REFLECTION");
      set_vertex_shader_define("PLANAR_REFLECTION");
      set_input(ShaderInput("reflectionRTT", ta->get_on_texture(stage)));

    } else if (stage_name == "lightmap") {
      set_pixel_shader_define("FLAT_LIGHTMAP");
      set_vertex_shader_define("FLAT_LIGHTMAP");
      set_input(ShaderInput("lightmapSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "lightmap_bumped") {
      set_pixel_shader_define("BUMPED_LIGHTMAP");
      set_vertex_shader_define("BUMPED_LIGHTMAP");
      set_input(ShaderInput("lightmapSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "bumpmap") {
      set_pixel_shader_define("BUMPMAP");
      set_vertex_shader_define("BUMPMAP");
      set_input(ShaderInput("bumpSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "envmap") {
      set_pixel_shader_define("ENVMAP");
      set_vertex_shader_define("ENVMAP");
      set_input(ShaderInput("envmapSampler", ta->get_on_texture(stage)));
    }
  }

}

#endif
