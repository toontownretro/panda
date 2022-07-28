/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceLightmappedShader.cxx
 * @author brian
 * @date 2022-03-17
 */

#include "sourceLightmappedShader.h"
#include "lightAttrib.h"
#include "cascadeLight.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamVector.h"
#include "materialParamFloat.h"
#include "materialParamColor.h"
#include "shaderAttrib.h"
#include "textureAttrib.h"
#include "texturePool.h"
#include "texture.h"
#include "textureStage.h"
#include "shaderManager.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"
#include "geomVertexAnimationSpec.h"
#include "fogAttrib.h"
#include "fog.h"
#include "alphaTestAttrib.h"
#include "textureStagePool.h"
#include "clipPlaneAttrib.h"

TypeHandle SourceLightmappedShader::_type_handle;

/**
 * Returns a dummy four-channel 1x1 white texture.
 */
static Texture *
get_white_texture() {
  static PT(Texture) tex = nullptr;
  if (tex == nullptr) {
    tex = new Texture("white");
    tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
    tex->set_minfilter(SamplerState::FT_nearest);
    tex->set_magfilter(SamplerState::FT_nearest);
    PTA_uchar image;
    image.push_back(255);
    image.push_back(255);
    image.push_back(255);
    image.push_back(255);
    tex->set_ram_image(image);
  }
  return tex;
}

/**
 * Synthesizes a shader for a given render state.
 */
void SourceLightmappedShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  // Combo names.
  static const CPT_InternalName IN_FOG("FOG");
  static const CPT_InternalName IN_ALPHA_TEST("ALPHA_TEST");
  static const CPT_InternalName IN_SUNLIGHT("SUNLIGHT");
  static const CPT_InternalName IN_SELFILLUM("SELFILLUM");
  static const CPT_InternalName IN_BUMPMAP("BUMPMAP");
  static const CPT_InternalName IN_ENVMAP("ENVMAP");
  static const CPT_InternalName IN_PLANAR_REFLECTION("PLANAR_REFLECTION");
  static const CPT_InternalName IN_ENVMAPMASK("ENVMAPMASK");
  static const CPT_InternalName IN_BASETEXTURE2("BASETEXTURE2");
  static const CPT_InternalName IN_BUMPMAP2("BUMPMAP2");
  static const CPT_InternalName IN_CLIPPING("CLIPPING");

  // Specialization constant names.
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");
  static const CPT_InternalName IN_BASEALPHAENVMAPMASK("BASEALPHAENVMAPMASK");
  static const CPT_InternalName IN_NORMALMAPALPHAENVMAPMASK("NORMALMAPALPHAENVMAPMASK");
  static const CPT_InternalName IN_SSBUMP("SSBUMP");
  static const CPT_InternalName IN_NUM_CASCADES("NUM_CASCADES");
  static const CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/source_lightmapped.vert.sho.pz");
  set_pixel_shader("shaders/source_lightmapped.frag.sho.pz");

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      set_pixel_shader_combo(IN_CLIPPING, 1);
      set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }

  const AlphaTestAttrib *at;
  if (state->get_attrib(at)) {
    if (at->get_mode() != AlphaTestAttrib::M_none &&
        at->get_mode() != AlphaTestAttrib::M_always) {
      set_pixel_shader_combo(IN_ALPHA_TEST, 1);
      // Specialize the pixel shader with the alpha test mode and
      // reference alpha, rather than using uniforms or the like.
      // Same is done for fog mode and clip plane count.
      set_spec_constant(IN_ALPHA_TEST_MODE, (int)at->get_mode());
      set_spec_constant(IN_ALPHA_TEST_REF, at->get_reference_alpha());
    }
  }

  const FogAttrib *fa;
  if (state->get_attrib(fa)) {
    Fog *fog = fa->get_fog();
    if (fog != nullptr) {
      set_pixel_shader_combo(IN_FOG, 1);
      set_spec_constant(IN_FOG_MODE, (int)fog->get_mode());
    }
  }

  MaterialParamBase *param;

  if ((param = material->get_param("base_color")) != nullptr) {
    set_input(ShaderInput("baseTexture", DCAST(MaterialParamTexture, param)->get_value()));
  } else {
    set_input(ShaderInput("baseTexture", get_white_texture()));
  }

  if ((param = material->get_param("basetexture2")) != nullptr) {
    set_pixel_shader_combo(IN_BASETEXTURE2, 1);
    set_input(ShaderInput("baseTexture2", DCAST(MaterialParamTexture, param)->get_value()));
  }

  bool has_bump = false;
  if ((param = material->get_param("bumpmap")) != nullptr) {
    has_bump = true;
    set_pixel_shader_combo(IN_BUMPMAP, 1);
    set_input(ShaderInput("normalTexture", DCAST(MaterialParamTexture, param)->get_value()));
  }
  if ((param = material->get_param("bumpmap2")) != nullptr) {
    has_bump = true;
    set_pixel_shader_combo(IN_BUMPMAP2, 1);
    set_input(ShaderInput("normalTexture2", DCAST(MaterialParamTexture, param)->get_value()));
  }
  if (has_bump && (param = material->get_param("ssbump")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
    set_spec_constant(IN_SSBUMP, true);
  }

  Texture *envmap_tex = nullptr;
  Texture *planar_tex = nullptr;
  bool env_cubemap = false;
  bool has_envmap = false;
  if ((param = material->get_param("envmap")) != nullptr) {
    if (param->get_type() == MaterialParamTexture::get_class_type()) {
      envmap_tex = DCAST(MaterialParamTexture, param)->get_value();

    } else if (param->get_type() == MaterialParamBool::get_class_type() &&
               DCAST(MaterialParamBool, param)->get_value()) {
      env_cubemap = true;
    }
  }

  const TextureAttrib *tattr;
  state->get_attrib_def(tattr);

  static TextureStage *lm_stage = TextureStagePool::get_stage(new TextureStage("lightmap"));
  static TextureStage *envmap_stage = TextureStagePool::get_stage(new TextureStage("envmap"));
  static TextureStage *planar_stage = TextureStagePool::get_stage(new TextureStage("reflection"));

  Texture *lm_tex = tattr->get_on_texture(lm_stage);
  if (lm_tex != nullptr) {
    set_input(ShaderInput("lightmapTexture", lm_tex));
  }
  if (env_cubemap) {
    envmap_tex = tattr->get_on_texture(envmap_stage);
  }
  if ((param = material->get_param("planarreflection")) != nullptr &&
      DCAST(MaterialParamBool, param)->get_value()) {
    planar_tex = tattr->get_on_texture(planar_stage);
  }

  if (env_cubemap && envmap_tex == nullptr) {
    envmap_tex = ShaderManager::get_global_ptr()->get_default_cube_map();
  }

  if (envmap_tex != nullptr || planar_tex != nullptr) {

    if (envmap_tex != nullptr) {
      set_pixel_shader_combo(IN_ENVMAP, 1);
      set_input(ShaderInput("envmapTexture", envmap_tex));

    } else {
      set_vertex_shader_combo(IN_PLANAR_REFLECTION, 1);
      set_pixel_shader_combo(IN_PLANAR_REFLECTION, 1);
      set_input(ShaderInput("reflectionSampler", planar_tex));
    }

    if ((param = material->get_param("envmapmask")) != nullptr) {
      set_pixel_shader_combo(IN_ENVMAPMASK, 1);
      set_input(ShaderInput("envmapMaskTexture", DCAST(MaterialParamTexture, param)->get_value()));
    }

    if ((param = material->get_param("basealphaenvmapmask")) != nullptr &&
         DCAST(MaterialParamBool, param)->get_value()) {
      set_spec_constant(IN_BASEALPHAENVMAPMASK, true);

    } else if ((param = material->get_param("normalmapalphaenvmapmask")) != nullptr &&
               DCAST(MaterialParamBool, param)->get_value()) {
      set_spec_constant(IN_NORMALMAPALPHAENVMAPMASK, true);
    }

    LVecBase3 envmap_tint(1.0f);
    if ((param = material->get_param("envmaptint")) != nullptr) {
      envmap_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    set_input(ShaderInput("envmapTint", envmap_tint));
    set_input(ShaderInput("envmapContrast", LVecBase3(1)));
    set_input(ShaderInput("envmapSaturation", LVecBase3(1)));
  }

  if ((param = material->get_param("selfillum")) != nullptr &&
      DCAST(MaterialParamBool, param)->get_value()) {
    set_pixel_shader_combo(IN_SELFILLUM, 1);

    LVecBase3 selfillum_tint(1.0f);
    if ((param = material->get_param("selfillumtint")) != nullptr) {
      selfillum_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    set_input(ShaderInput("selfIllumTint", selfillum_tint));
  }

  const LightAttrib *la;
  state->get_attrib_def(la);
  if (!la->has_all_off() && la->get_num_non_ambient_lights() == 1) {
    const NodePath &light = la->get_on_light_quick(0);
    if (light.node()->get_type() == CascadeLight::get_class_type()) {
      CascadeLight *clight = DCAST(CascadeLight, light.node());
      if (clight->is_shadow_caster()) {
        set_vertex_shader_combo(IN_SUNLIGHT, 1);
        set_pixel_shader_combo(IN_SUNLIGHT, 1);
        set_spec_constant(IN_NUM_CASCADES, clight->get_num_cascades());
      }
    }
  }
}
