/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexLitShader.cxx
 * @author lachbr
 * @date 2020-10-30
 */

#include "vertexLitShader.h"
#include "lightAttrib.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "postProcessDefines.h"
#include "shaderParamAttrib.h"

TypeHandle VertexLitShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void VertexLitShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const ShaderParamAttrib *params,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/vertexLitGeneric_PBR.vert.glsl");
  set_pixel_shader("shaders/vertexLitGeneric_PBR.frag.glsl");

  bool need_tbn = true;
  bool need_world_position = true;
  bool need_world_normal = true;
  bool need_world_vec = true;
  bool need_eye_position = false;

  add_shader_quality();
  add_transparency(state);
  add_alpha_test(state);
  add_hdr(state);

  int aux = add_aux_attachments(state);
  if ((aux & AUXTEXTUREBITS_NORMAL) != 0) {
    need_world_normal = true;
  }

  // Break out the lights by type.
  const LightAttrib *la;
  state->get_attrib_def(la);
  size_t num_lights = la->get_num_non_ambient_lights();
  if (num_lights > 0) {
    need_world_vec = true;
    need_world_normal = true;

    set_pixel_shader_define("LIGHTING");
    set_pixel_shader_define("NUM_LIGHTS", num_lights);
    set_vertex_shader_define("NUM_LIGHTS", num_lights);
  }

  // Are we self-illuminating?
  int selfillum_idx = params->find_param("selfillum");
  if (selfillum_idx != -1) {
    bool selfillum = (bool)atoi(params->get_param_value(selfillum_idx).c_str());
    if (selfillum) {
      set_pixel_shader_define("SELFILLUM");
      set_input(ShaderInput("selfillumTint", LVecBase3(1.0)));
    }
  }

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
      set_vertex_shader_define("BASETEXTURE_INDEX", i);
      set_input(ShaderInput("baseTextureSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "reflection") {
      set_pixel_shader_define("PLANAR_REFLECTION");
      set_vertex_shader_define("PLANAR_REFLECTION");
      set_input(ShaderInput("reflectionSampler", ta->get_on_texture(stage)));

    }
  }

  set_vertex_shader_define("NUM_TEXTURES", num_stages);

  if (add_csm(state)) {
    need_world_normal = true;
    need_world_position = true;
  }

  if (add_clip_planes(state)) {
    need_world_position = true;
  }

  if (add_fog(state)) {
    need_eye_position = true;
  }

  add_hardware_skinning(anim_spec);

  if (need_world_vec) {
    need_world_position = true;
  }

  if (need_tbn) {
    set_vertex_shader_define("NEED_TBN");
    set_pixel_shader_define("NEED_TBN");
  }

  if (need_world_normal) {
    set_vertex_shader_define("NEED_WORLD_NORMAL");
    set_pixel_shader_define("NEED_WORLD_NORMAL");
  }

  if (need_world_position) {
    set_vertex_shader_define("NEED_WORLD_POSITION");
    set_pixel_shader_define("NEED_WORLD_POSITION");
  }

  if (need_eye_position) {
    set_vertex_shader_define("NEED_EYE_POSITION");
    set_pixel_shader_define("NEED_EYE_POSITION");
  }

  if (need_world_vec) {
    set_vertex_shader_define("NEED_WORLD_VEC");
    set_pixel_shader_define("NEED_WORLD_VEC");
  }
}
