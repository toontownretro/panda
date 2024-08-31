/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pbrShader.cxx
 * @author brian
 * @date 2024-08-30
 */

#include "pbrShader.h"
#include "lightAttrib.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamVector.h"
#include "materialParamFloat.h"
#include "materialParamInt.h"
#include "materialParamColor.h"
#include "materialParamMatrix.h"
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
#include "cascadeLight.h"
#include "renderState.h"
#include "textureStagePool.h"
#include "clipPlaneAttrib.h"
#include "texMatrixAttrib.h"
#include "config_shader.h"

TypeHandle PBRShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void PBRShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  // Combo names.
  static const CPT_InternalName IN_SKINNING("SKINNING");
  static const CPT_InternalName IN_FOG("FOG");
  static const CPT_InternalName IN_ALPHA_TEST("ALPHA_TEST");
  static const CPT_InternalName IN_DIRECT_LIGHT("DIRECT_LIGHT");
  static const CPT_InternalName IN_AMBIENT_LIGHT("AMBIENT_LIGHT");
  static const CPT_InternalName IN_ENVMAP("ENVMAP");
  static const CPT_InternalName IN_HAS_SHADOW_SUNLIGHT("HAS_SHADOW_SUNLIGHT");
  static const CPT_InternalName IN_CLIPPING("CLIPPING");
  static const CPT_InternalName IN_LIGHTMAP("LIGHTMAP");

  // Specialization constant names.
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");
  static const CPT_InternalName IN_NUM_LIGHTS("NUM_LIGHTS");
  static const CPT_InternalName IN_NUM_CASCADES("NUM_CASCADES");
  static const CPT_InternalName IN_CSM_LIGHT_ID("CSM_LIGHT_ID");
  static const CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");
  static const CPT_InternalName IN_BAKED_VERTEX_LIGHT("BAKED_VERTEX_LIGHT");
  static const CPT_InternalName IN_BLEND_MODE("BLEND_MODE");

  ShaderManager *mgr = ShaderManager::get_global_ptr();

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/pbr.vert.sho.pz");
  setup.set_pixel_shader("shaders/pbr.frag.sho.pz");

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      setup.set_pixel_shader_combo(IN_CLIPPING, 1);
      setup.set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }

  // Toggle GPU skinning.
  const ShaderAttrib *sa;
  state->get_attrib_def(sa);
  if (sa->has_hardware_skinning()) {
    if (sa->get_num_transforms() > 4) {
      // 8 transforms version.
      setup.set_vertex_shader_combo(IN_SKINNING, 2);
    } else {
      setup.set_vertex_shader_combo(IN_SKINNING, 1);
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

  // Break out the lights by type.
  const LightAttrib *la;
  state->get_attrib_def(la);
  size_t num_lights = la->has_all_off() ? 0 : la->get_num_non_ambient_lights();

  bool has_ambient_probe = false;

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
    setup.set_vertex_shader_combo(IN_LIGHTMAP, 1);
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
  else if (!la->has_all_off()) {
    if (sa->has_shader_input("ambientProbe")) {
      setup.set_pixel_shader_combo(IN_AMBIENT_LIGHT, 2);
      has_ambient_probe = true;
    } else {
      size_t num_ambient_lights = la->get_num_on_lights() - num_lights;
      if (num_ambient_lights != 0) {
        setup.set_pixel_shader_combo(IN_AMBIENT_LIGHT, 1);
      }
    }
  }

  if (sa->has_shader_input("bakedVertexLight")) {
    setup.set_spec_constant(IN_BAKED_VERTEX_LIGHT, true);
  } else {
    setup.set_spec_constant(IN_BAKED_VERTEX_LIGHT, false);
  }

  bool has_direct_light = (num_lights != 0u);

  if (has_direct_light) {
    setup.set_pixel_shader_combo(IN_DIRECT_LIGHT, 1);
    setup.set_spec_constant(IN_NUM_LIGHTS, (unsigned int)std::min(num_lights, (size_t)4u));

    // See if we have a shadow casting CascadeLight.
    for (size_t i = 0; i < num_lights; ++i) {
      const NodePath &np = la->get_on_light_quick(i);
      if (np.node()->get_type() == CascadeLight::get_class_type()) {
        CascadeLight *clight = DCAST(CascadeLight, np.node());
        if (clight->is_shadow_caster()) {
          // Sunlight shadows are enabled!
          setup.set_vertex_shader_combo(IN_HAS_SHADOW_SUNLIGHT, 1);
          setup.set_pixel_shader_combo(IN_HAS_SHADOW_SUNLIGHT, 1);
          setup.set_spec_constant(IN_CSM_LIGHT_ID, (int)i);
          setup.set_spec_constant(IN_NUM_CASCADES, clight->get_num_cascades());
          Texture *shofs_tex = mgr->get_shadow_offset_texture();
          setup.set_input(ShaderInput("shadowOffsetTexture", shofs_tex, shofs_tex->get_default_sampler()));
          setup.set_input(ShaderInput("shadowOffsetParams", LVecBase4((float)shadow_pcss_softness,
                                                                    (float)shadow_offset_window_size,
                                                                    (float)shadow_offset_filter_size,
                                                                    (float)shadow_pcss_light_size)));
        }
        break;
      }
    }
  }

  MaterialParamBase *param;

  if ((param = material->get_param("base_color")) != nullptr) {
    setup.set_input(ShaderInput("albedo_sampler", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  } else {
    setup.set_input(ShaderInput("albedo_sampler", mgr->get_white_texture()));
  }

  bool has_bump = false;
  if ((param = material->get_param("normal")) != nullptr) {
    has_bump = true;
    setup.set_input(ShaderInput("normal_sampler", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  } else {
    setup.set_input(ShaderInput("normal_sampler", mgr->get_flat_normal_map()));
  }

  if ((param = material->get_param("roughness")) != nullptr) {
    setup.set_input(ShaderInput("roughness_sampler", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  } else {
    setup.set_input(ShaderInput("roughness_sampler", mgr->get_white_texture()));
  }

  if ((param = material->get_param("metalness")) != nullptr) {
    setup.set_input(ShaderInput("metalness_sampler", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  } else {
    setup.set_input(ShaderInput("metalness_sampler", mgr->get_black_texture()));
  }

  if ((param = material->get_param("ao")) != nullptr) {
    setup.set_input(ShaderInput("ao_sampler", DCAST(MaterialParamTexture, param)->get_value(), DCAST(MaterialParamTexture, param)->get_sampler_state()));
  } else {
    setup.set_input(ShaderInput("ao_sampler", mgr->get_white_texture()));
  }

  LVecBase4 scales(1.0f, 1.0f, 1.0f, 1.0f);
  if ((param = material->get_param("roughness_scale")) != nullptr) {
    scales[0] = DCAST(MaterialParamFloat, param)->get_value();
  }
  if ((param = material->get_param("ao_scale")) != nullptr) {
    scales[1] = DCAST(MaterialParamFloat, param)->get_value();
  }
  if ((param = material->get_param("emission_scale")) != nullptr) {
    scales[2] = DCAST(MaterialParamFloat, param)->get_value();
  }
  if ((param = material->get_param("normal_scale")) != nullptr) {
    scales[3] = DCAST(MaterialParamFloat, param)->get_value();
  }
  setup.set_input(ShaderInput("scales", scales));

  Texture *envmap_tex = nullptr;
  SamplerState envmap_samp;
  bool env_cubemap = false;

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

    if (env_cubemap && envmap_tex == nullptr) {
      envmap_tex = mgr->get_default_cube_map();
      envmap_samp = envmap_tex->get_default_sampler();
    }
  }

  if (envmap_tex != nullptr) {
    setup.set_pixel_shader_combo(IN_ENVMAP, 1);
    setup.set_input(ShaderInput("cubemap_sampler", envmap_tex, envmap_samp));
    setup.set_input(ShaderInput("specular_brdf_lut", TexturePool::load_texture("maps/brdf_lut.txo")));
  }
}
