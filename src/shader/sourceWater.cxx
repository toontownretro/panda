/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceWater.cxx
 * @author brian
 * @date 2022-07-10
 */

#include "sourceWater.h"
#include "material.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "texture.h"
#include "textureStage.h"
#include "textureStagePool.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamFloat.h"
#include "materialParamVector.h"
#include "shaderManager.h"

TypeHandle SourceWater::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void SourceWater::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  static CPT_InternalName IN_ANIMATEDNORMALMAP("ANIMATEDNORMALMAP");
  static CPT_InternalName IN_FOG("FOG");

  ShaderManager *mgr = ShaderManager::get_global_ptr();

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/source_water.vert.sho.pz");
  setup.set_pixel_shader("shaders/source_water.frag.sho.pz");

  //static TextureStage *lm_stage = TextureStagePool::get_stage(new TextureStage("lightmap"));
  static TextureStage *refl_stage = TextureStagePool::get_stage(new TextureStage("reflection"));
  static TextureStage *refr_stage = TextureStagePool::get_stage(new TextureStage("refraction"));
  static TextureStage *refr_depth_stage = TextureStagePool::get_stage(new TextureStage("refraction_depth"));

  const TextureAttrib *ta;
  state->get_attrib_def(ta);

  //Texture *lm_tex = ta->get_on_texture(lm_stage);
  //if (lm_tex == nullptr) {
  //  lm_tex = mgr->get_black_lightmap();
  //}
  //nassertv(lm_tex != nullptr);

  Texture *refl_tex = ta->get_on_texture(refl_stage);
  if (refl_tex == nullptr) {
    refl_tex = mgr->get_black_texture();
  }
  Texture *refr_tex = ta->get_on_texture(refr_stage);
  if (refr_tex == nullptr) {
    refr_tex = mgr->get_black_texture();
  }

  //setup.set_input(ShaderInput("lightmapSampler", lm_tex, lm_tex->get_default_sampler()));
  setup.set_input(ShaderInput("reflectionSampler", refl_tex, refl_tex->get_default_sampler()));
  setup.set_input(ShaderInput("refractionSampler", refr_tex, refr_tex->get_default_sampler()));

  MaterialParamBase *param;

  Texture *refr_depth_tex = ta->get_on_texture(refr_depth_stage);
  if ((refr_depth_tex != nullptr) &&
      (param = material->get_param("fog")) != nullptr &&
      DCAST(MaterialParamBool, param)->get_value()) {

    setup.set_pixel_shader_combo(IN_FOG, 1);
    setup.set_input(ShaderInput("refractionDepthSampler", refr_depth_tex, refr_depth_tex->get_default_sampler()));

    LVecBase3 fog_color(0.5f);
    float fog_density = 1.0f;

    if ((param = material->get_param("fogcolor")) != nullptr) {
      fog_color = DCAST(MaterialParamVector, param)->get_value() / 255.0f;
    }
    if ((param = material->get_param("fogdensity")) != nullptr) {
      fog_density = DCAST(MaterialParamFloat, param)->get_value();
    }

    setup.set_input(ShaderInput("u_fogColor_density", LVecBase4(fog_color, fog_density * 0.01f)));
  }

  if ((param = material->get_param("base_color")) != nullptr) {
    Texture *norm_tex = DCAST(MaterialParamTexture, param)->get_value();
    SamplerState norm_samp = DCAST(MaterialParamTexture, param)->get_sampler_state();
    setup.set_input(ShaderInput("normalSampler", norm_tex, norm_samp));

    if ((norm_tex->get_texture_type() == Texture::TT_2d_texture_array) &&
        ((param = material->get_param("animatednormalmap")) != nullptr) &&
        DCAST(MaterialParamBool, param)->get_value()) {

      if ((param = material->get_param("interpnormalframes")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
        // Animated normal map with frame interp.
        setup.set_pixel_shader_combo(IN_ANIMATEDNORMALMAP, 2);
      } else {
        // Animated normal map with no frame interp.
        setup.set_pixel_shader_combo(IN_ANIMATEDNORMALMAP, 1);
      }

      float fps = 24.0f;
      if ((param = material->get_param("normalmapfps")) != nullptr) {
        fps = DCAST(MaterialParamFloat, param)->get_value();
      }

      setup.set_input(ShaderInput("u_normalMapFPS", LVecBase2(fps)));
    }

  } else {
    setup.set_input(ShaderInput("normalSampler", mgr->get_flat_normal_map()));
  }

  // Scaling of the normal map distortion for reflection and refraction
  // respectively.
  float reflect_scale = 1.0f;
  if ((param = material->get_param("reflectnormalscale")) != nullptr) {
    reflect_scale = DCAST(MaterialParamFloat, param)->get_value();
  }
  float refract_scale = 1.0f;
  if ((param = material->get_param("refractnormalscale")) != nullptr) {
    refract_scale = DCAST(MaterialParamFloat, param)->get_value();
  }
  setup.set_input(ShaderInput("u_reflectRefractScale", LVecBase4(reflect_scale, reflect_scale, refract_scale, refract_scale)));

  LVecBase3 reflect_tint(1.0f);
  if ((param = material->get_param("reflecttint")) != nullptr) {
    reflect_tint = DCAST(MaterialParamVector, param)->get_value();
  }
  setup.set_input(ShaderInput("u_reflectTint", reflect_tint));

  LVecBase3 refract_tint(1.0f);
  if ((param = material->get_param("refracttint")) != nullptr) {
    refract_tint = DCAST(MaterialParamVector, param)->get_value();
  }
  setup.set_input(ShaderInput("u_refractTint", refract_tint));

  float fresnel_exp = 5.0f;
  if ((param = material->get_param("fresnelexponent")) != nullptr) {
    fresnel_exp = DCAST(MaterialParamFloat, param)->get_value();
  }
  setup.set_input(ShaderInput("u_fresnelExponent", LVecBase2(fresnel_exp)));
}
