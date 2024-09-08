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
#include "materialParamInt.h"
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
#include "config_shader.h"

TypeHandle SourceLightmappedShader::_type_handle;

/**
 * Returns a dummy four-channel 1x1 white texture.
 */
static Texture *
sls_get_white_texture() {
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
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

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
  static const CPT_InternalName IN_DETAIL("DETAIL");
  static const CPT_InternalName IN_LIGHTMAP("LIGHTMAP");

  // Specialization constant names.
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");
  static const CPT_InternalName IN_BASEALPHAENVMAPMASK("BASEALPHAENVMAPMASK");
  static const CPT_InternalName IN_NORMALMAPALPHAENVMAPMASK("NORMALMAPALPHAENVMAPMASK");
  static const CPT_InternalName IN_SSBUMP("SSBUMP");
  static const CPT_InternalName IN_NUM_CASCADES("NUM_CASCADES");
  static const CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");
  static const CPT_InternalName IN_BLEND_MODE("BLEND_MODE");
  static const CPT_InternalName IN_DETAIL_BLEND_MODE("DETAIL_BLEND_MODE");

  ShaderManager *mgr = ShaderManager::get_global_ptr();

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/source_lightmapped.vert.sho.pz");
  setup.set_pixel_shader("shaders/source_lightmapped.frag.sho.pz");

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      setup.set_pixel_shader_combo(IN_CLIPPING, 1);
      setup.set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }

  const AlphaTestAttrib *at;
  if (state->get_attrib(at)) {
    if (at->get_mode() != AlphaTestAttrib::M_none &&
        at->get_mode() != AlphaTestAttrib::M_always) {
      setup.set_pixel_shader_combo(IN_ALPHA_TEST, 1);
      // Specialize the pixel shader with the alpha test mode and
      // reference alpha, rather than using uniforms or the like.
      // Same is done for fog mode and clip plane count.
      setup.set_spec_constant(IN_ALPHA_TEST_MODE, (int)at->get_mode());
      setup.set_spec_constant(IN_ALPHA_TEST_REF, at->get_reference_alpha());
    }
  }

  const FogAttrib *fa;
  if (state->get_attrib(fa)) {
    Fog *fog = fa->get_fog();
    if (fog != nullptr) {
      setup.set_pixel_shader_combo(IN_FOG, 1);
      setup.set_spec_constant(IN_FOG_MODE, (int)fog->get_mode());
      if (has_additive_blend(state)) {
        setup.set_spec_constant(IN_BLEND_MODE, 2);
      } else if (has_modulate_blend(state)) {
        setup.set_spec_constant(IN_BLEND_MODE, 1);
      }
    }
  }

  MaterialParamBase *param;

  if ((param = material->get_param("base_color")) != nullptr) {
    setup.set_input(ShaderInput("baseTexture", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  } else {
    setup.set_input(ShaderInput("baseTexture", mgr->get_white_texture()));
  }

  if ((param = material->get_param("basetexture2")) != nullptr) {
    setup.set_pixel_shader_combo(IN_BASETEXTURE2, 1);
    setup.set_input(ShaderInput("baseTexture2", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  }

  bool has_bump = false;
  if ((param = material->get_param("bumpmap")) != nullptr) {
    has_bump = true;
    setup.set_pixel_shader_combo(IN_BUMPMAP, 1);
    setup.set_input(ShaderInput("normalTexture", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  }
  if ((param = material->get_param("bumpmap2")) != nullptr) {
    has_bump = true;
    setup.set_pixel_shader_combo(IN_BUMPMAP2, 1);
    setup.set_input(ShaderInput("normalTexture2", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  }
  if (has_bump && (param = material->get_param("ssbump")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
    setup.set_spec_constant(IN_SSBUMP, true);
  }

  const TextureAttrib *tattr;
  state->get_attrib_def(tattr);

  static TextureStage *lm_stage = TextureStagePool::get_stage(new TextureStage("lightmap"));
  static TextureStage *lm_stage_l1y = TextureStagePool::get_stage(new TextureStage("lightmap_l1y"));
  static TextureStage *lm_stage_l1z = TextureStagePool::get_stage(new TextureStage("lightmap_l1z"));
  static TextureStage *lm_stage_l1x = TextureStagePool::get_stage(new TextureStage("lightmap_l1x"));
  static TextureStage *envmap_stage = TextureStagePool::get_stage(new TextureStage("envmap"));
  static TextureStage *planar_stage = TextureStagePool::get_stage(new TextureStage("reflection"));

  Texture *lm_tex = tattr->get_on_texture(lm_stage);
  if (lm_tex != nullptr) {
    setup.set_pixel_shader_combo(IN_LIGHTMAP, 1);
    setup.set_input(ShaderInput("lightmapTextureL0", lm_tex, tattr->get_on_sampler(lm_stage)));
    Texture *lm_tex_l1y = tattr->get_on_texture(lm_stage_l1y);
    if (lm_tex_l1y != nullptr) {
      setup.set_input(ShaderInput("lightmapTextureL1y", lm_tex_l1y, tattr->get_on_sampler(lm_stage_l1y)));
    }
    Texture *lm_tex_l1z = tattr->get_on_texture(lm_stage_l1z);
    if (lm_tex_l1z != nullptr) {
      setup.set_input(ShaderInput("lightmapTextureL1z", lm_tex_l1z, tattr->get_on_sampler(lm_stage_l1z)));
    }
    Texture *lm_tex_l1x = tattr->get_on_texture(lm_stage_l1x);
    if (lm_tex_l1x != nullptr) {
      setup.set_input(ShaderInput("lightmapTextureL1x", lm_tex_l1x, tattr->get_on_sampler(lm_stage_l1x)));
    }
  }

  Texture *envmap_tex = nullptr;
  Texture *planar_tex = nullptr;
  SamplerState envmap_samp, planar_samp;
  bool env_cubemap = false;
  bool has_envmap = false;

  if (cubemaps_enabled) {
    if ((param = material->get_param("envmap")) != nullptr) {
      if (param->get_type() == MaterialParamTexture::get_class_type()) {
        envmap_tex = DCAST(MaterialParamTexture, param)->get_value();

      } else if (param->get_type() == MaterialParamBool::get_class_type() &&
                DCAST(MaterialParamBool, param)->get_value()) {
        env_cubemap = true;
      }
    }

    if (env_cubemap) {
      envmap_tex = tattr->get_on_texture(envmap_stage);
      envmap_samp = tattr->get_on_sampler(envmap_stage);
    }
    if ((param = material->get_param("planarreflection")) != nullptr &&
        DCAST(MaterialParamBool, param)->get_value()) {
      planar_tex = tattr->get_on_texture(planar_stage);
      planar_samp = tattr->get_on_sampler(planar_stage);
    }

    if (env_cubemap && envmap_tex == nullptr) {
      envmap_tex = ShaderManager::get_global_ptr()->get_default_cube_map();
      envmap_samp = envmap_tex->get_default_sampler();
    }
  }

  if (envmap_tex != nullptr || planar_tex != nullptr) {

    if (envmap_tex != nullptr) {
      setup.set_pixel_shader_combo(IN_ENVMAP, 1);
      setup.set_input(ShaderInput("envmapTexture", envmap_tex, envmap_samp));

    } else {
      setup.set_vertex_shader_combo(IN_PLANAR_REFLECTION, 1);
      setup.set_pixel_shader_combo(IN_PLANAR_REFLECTION, 1);
      setup.set_input(ShaderInput("reflectionSampler", planar_tex, envmap_samp));
    }

    if ((param = material->get_param("envmapmask")) != nullptr) {
      setup.set_pixel_shader_combo(IN_ENVMAPMASK, 1);
      setup.set_input(ShaderInput("envmapMaskTexture", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
    }

    if ((param = material->get_param("basealphaenvmapmask")) != nullptr &&
         DCAST(MaterialParamBool, param)->get_value()) {
      setup.set_spec_constant(IN_BASEALPHAENVMAPMASK, true);

    } else if ((param = material->get_param("normalmapalphaenvmapmask")) != nullptr &&
               DCAST(MaterialParamBool, param)->get_value()) {
      setup.set_spec_constant(IN_NORMALMAPALPHAENVMAPMASK, true);
    }

    LVecBase3 envmap_tint(1.0f);
    if ((param = material->get_param("envmaptint")) != nullptr) {
      envmap_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    setup.set_input(ShaderInput("envmapTint", envmap_tint));
    setup.set_input(ShaderInput("envmapContrast", LVecBase3(1)));
    setup.set_input(ShaderInput("envmapSaturation", LVecBase3(1)));
  }

  if ((param = material->get_param("selfillum")) != nullptr &&
      DCAST(MaterialParamBool, param)->get_value()) {
    setup.set_pixel_shader_combo(IN_SELFILLUM, 1);

    LVecBase3 selfillum_tint(1.0f);
    if ((param = material->get_param("selfillumtint")) != nullptr) {
      selfillum_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    setup.set_input(ShaderInput("selfIllumTint", selfillum_tint));
  }

  // Detail texture.
  if ((param = material->get_param("detail")) != nullptr) {
    setup.set_pixel_shader_combo(IN_DETAIL, 1);

    Texture *detail_tex = DCAST(MaterialParamTexture, param)->get_value();
    const SamplerState &detail_samp = DCAST(MaterialParamTexture, param)->get_sampler_state();

    LVecBase2 params(1.0f, 4.0f);
    if ((param = material->get_param("detailblendfactor")) != nullptr) {
      params[0] = DCAST(MaterialParamFloat, param)->get_value();
    }
    if ((param = material->get_param("detailscale")) != nullptr) {
      params[1] = DCAST(MaterialParamFloat, param)->get_value();
    }
    LVecBase3 detail_tint(1.0f);
    if ((param = material->get_param("detailtint")) != nullptr) {
      detail_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    int blend_mode = 0;
    if ((param = material->get_param("detailblendmode")) != nullptr) {
      blend_mode = DCAST(MaterialParamInt, param)->get_value();
    }

    setup.set_input(ShaderInput("detailSampler", detail_tex, detail_samp));
    setup.set_input(ShaderInput("detailParams", params));
    setup.set_input(ShaderInput("detailTint", detail_tint));
    setup.set_spec_constant(IN_DETAIL_BLEND_MODE, blend_mode);
  }

  const LightAttrib *la;
  state->get_attrib_def(la);
  if (!la->has_all_off() && la->get_num_non_ambient_lights() == 1) {
    const NodePath &light = la->get_on_light_quick(0);
    if (light.node()->get_type() == CascadeLight::get_class_type()) {
      CascadeLight *clight = DCAST(CascadeLight, light.node());
      if (clight->is_shadow_caster()) {
        setup.set_vertex_shader_combo(IN_SUNLIGHT, 1);
        setup.set_pixel_shader_combo(IN_SUNLIGHT, 2);
        setup.set_spec_constant(IN_NUM_CASCADES, clight->get_num_cascades());
        setup.set_input(ShaderInput("shadowOffsetTexture", mgr->get_shadow_offset_texture()));
        setup.set_input(ShaderInput("shadowOffsetParams", LVecBase4((float)shadow_pcss_softness,
                                                                    (float)shadow_offset_window_size,
                                                                    (float)shadow_offset_filter_size,
                                                                    (float)shadow_pcss_light_size)));
      } else {
        setup.set_pixel_shader_combo(IN_SUNLIGHT, 1);
      }
    } else if (light.node()->get_type() == DirectionalLight::get_class_type()) {
      setup.set_pixel_shader_combo(IN_SUNLIGHT, 1);
    }
  }
}
