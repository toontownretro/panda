/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceShader.cxx
 * @author brian
 * @date 2021-10-25
 */

#include "sourceShader.h"
#include "lightAttrib.h"
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

static ConfigVariableBool use_orig_source_shader
("use-orig-source-shader", false);

static ConfigVariableDouble remap_param0("remap-param-0", 0.5);
static ConfigVariableDouble remap_param1("remap-param-1", 0.5);

TypeHandle SourceShader::_type_handle;

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
 * Returns a dummy four-channel 1x1 black texture.
 */
static Texture *
get_black_texture() {
  static PT(Texture) tex = nullptr;
  if (tex == nullptr) {
    tex = new Texture("black");
    tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
    tex->set_minfilter(SamplerState::FT_nearest);
    tex->set_magfilter(SamplerState::FT_nearest);
    PTA_uchar image;
    image.push_back(0);
    image.push_back(0);
    image.push_back(0);
    image.push_back(0);
    tex->set_ram_image(image);
  }
  return tex;
}

/**
 * Returns a flat 1x1 normal map.
 */
static Texture *
get_flat_normal_map() {
  static PT(Texture) tex = nullptr;
  if (tex == nullptr) {
    tex = new Texture("flat_normal");
    tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
    tex->set_minfilter(SamplerState::FT_nearest);
    tex->set_magfilter(SamplerState::FT_nearest);
    PTA_uchar image;
    image.push_back(128);
    image.push_back(128);
    image.push_back(255);
    image.push_back(255);
    tex->set_ram_image_as(image, "RGBA");
  }
  return tex;
}

/**
 * Synthesizes a shader for a given render state.
 */
void SourceShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  // Combo names.
  static const CPT_InternalName IN_SKINNING("SKINNING");
  static const CPT_InternalName IN_FOG("FOG");
  static const CPT_InternalName IN_ALPHA_TEST("ALPHA_TEST");
  static const CPT_InternalName IN_DIRECT_LIGHT("DIRECT_LIGHT");
  static const CPT_InternalName IN_AMBIENT_LIGHT("AMBIENT_LIGHT");
  static const CPT_InternalName IN_PHONG("PHONG");
  static const CPT_InternalName IN_PHONGWARP("PHONGWARP");
  static const CPT_InternalName IN_LIGHTWARP("LIGHTWARP");
  static const CPT_InternalName IN_RIMLIGHT("RIMLIGHT");
  static const CPT_InternalName IN_SELFILLUM("SELFILLUM");
  static const CPT_InternalName IN_SELFILLUMMASK("SELFILLUMMASK");
  static const CPT_InternalName IN_BUMPMAP("BUMPMAP");
  static const CPT_InternalName IN_ENVMAP("ENVMAP");

  // Specialization constant names.
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");
  static const CPT_InternalName IN_NUM_LIGHTS("NUM_LIGHTS");
  static const CPT_InternalName IN_HALFLAMBERT("HALFLAMBERT");
  static const CPT_InternalName IN_BASEMAPALPHAENVMAPMASK("BASEMAPALPHAENVMAPMASK");
  static const CPT_InternalName IN_NORMALMAPALPHAENVMAPMASK("NORMALMAPALPHAENVMAPMASK");
  static const CPT_InternalName IN_PHONGEXPONENTFACTOR("PHONGEXPONENTFACTOR");
  static const CPT_InternalName IN_BASEMAPALPHAPHONGMASK("BASEMAPALPHAPHONGMASK");

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shadersnew/source_vlg.vert.sho.pz");
  //if (use_orig_source_shader) {
  //  set_pixel_shader("shaders/source_vlg_orig.frag.glsl");
  //} else {
  //  set_pixel_shader("shaders/source_vlg.frag.glsl");
  //}
  set_pixel_shader("shadersnew/source_vlg.frag.sho.pz");

  // Hardware skinning?
  if (anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      anim_spec.get_num_transforms() > 0) {
    set_vertex_shader_combo(IN_SKINNING, 1);
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

  SourceMaterial *src_mat = DCAST(SourceMaterial, material);

  // Break out the lights by type.
  const LightAttrib *la;
  state->get_attrib_def(la);
  size_t num_lights = la->get_num_non_ambient_lights();

  const ShaderAttrib *sa;
  state->get_attrib_def(sa);
  bool has_ambient_probe = false;
  if (sa->has_shader_input("ambientProbe")) {
    set_pixel_shader_combo(IN_AMBIENT_LIGHT, 2);
    has_ambient_probe = true;
  } else {
    size_t num_ambient_lights = la->get_num_on_lights() - num_lights;
    if (num_ambient_lights != 0) {
      set_pixel_shader_combo(IN_AMBIENT_LIGHT, 1);
    }
  }

  bool has_direct_light = (num_lights != 0u);

  if (has_direct_light) {
    set_pixel_shader_combo(IN_DIRECT_LIGHT, 1);
    set_spec_constant(IN_NUM_LIGHTS, (unsigned int)std::min(num_lights, (size_t)4u));
  }

  MaterialParamBase *param;

  if ((param = src_mat->get_param("base_color")) != nullptr) {
    set_input(ShaderInput("albedoTexture", DCAST(MaterialParamTexture, param)->get_value()));
  } else {
    set_input(ShaderInput("albedoTexture", get_white_texture()));
  }

  if (has_direct_light && (param = src_mat->get_param("lightwarptexture")) != nullptr) {
    set_pixel_shader_combo(IN_LIGHTWARP, 1);
    set_input(ShaderInput("lightWarpTexture", DCAST(MaterialParamTexture, param)->get_value()));
  }

  bool has_rimlight = false;
  if ((param = src_mat->get_param("phong")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
    // Phong enabled on material.
    set_pixel_shader_combo(IN_PHONG, 1);

    set_input(ShaderInput("remapParams", LVecBase2(remap_param0, remap_param1)));

    bool has_phong_exponent_texture = false;
    // Phong exponent texture?  This contains per-texel phong exponent in R,
    // per-texel mask of whether to tint phong by albedo in G, nothing in B,
    // and an optional rim lighting mask in A.
    if ((param = src_mat->get_param("phongexponenttexture")) != nullptr) {
      has_phong_exponent_texture = true;
      set_input(ShaderInput("phongExponentTexture", DCAST(MaterialParamTexture, param)->get_value()));

    } else {
      // If it wasn't specified, just use a default white texture.
      set_input(ShaderInput("phongExponentTexture", get_white_texture()));
    }

    // Exponent, albedo tint mask, boost
    LVecBase3 phong_params(-1.0f, 0.0f, 1.0f);
    if (!has_phong_exponent_texture && (param = src_mat->get_param("phongexponent")) != nullptr) {
      // This is only used if there isn't a dedicated per-texel phong exponent
      // map.
      phong_params[0] = DCAST(MaterialParamFloat, param)->get_value();
    }
    if ((param = src_mat->get_param("phongalbedotint")) != nullptr) {
      phong_params[1] = DCAST(MaterialParamBool, param)->get_value();
    }
    if ((param = src_mat->get_param("phongboost")) != nullptr) {
      phong_params[2] = DCAST(MaterialParamFloat, param)->get_value();
    }
    if (has_phong_exponent_texture && (param = src_mat->get_param("phongexponentfactor")) != nullptr) {
      // A factor was specified for the phong exponent map.  Use this as the phong exponent.
      set_spec_constant(IN_PHONGEXPONENTFACTOR, true);
      phong_params[0] = DCAST(MaterialParamFloat, param)->get_value();
    }
    set_input(ShaderInput("phongParams", phong_params));

    // Does material specify custom piece-wise fresnel?
    LVecBase3 phong_fresnel_ranges(0.0f, 0.5f, 1.0f);
    if ((param = src_mat->get_param("phongfresnelranges")) != nullptr) {
      phong_fresnel_ranges = DCAST(MaterialParamVector, param)->get_value();
    }
    set_input(ShaderInput("phongFresnelRanges", phong_fresnel_ranges));

    // Phong tint?
    LVecBase3 phong_tint(1.0f, 1.0f, 1.0f);
    if ((param = src_mat->get_param("phongtint")) != nullptr) {
      phong_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    set_input(ShaderInput("phongTint", phong_tint));

    // How about a phong warp texture?
    if (has_direct_light && (param = src_mat->get_param("phongwarptexture")) != nullptr) {
      set_pixel_shader_combo(IN_PHONGWARP, 1);
      set_input(ShaderInput("phongWarpTexture", DCAST(MaterialParamTexture, param)->get_value()));
    }

    if ((param = src_mat->get_param("basemapalphaphongmask")) != nullptr) {
      set_spec_constant(IN_BASEMAPALPHAPHONGMASK, true);
    }

    // Do we want rim lighting?
    if ((param = src_mat->get_param("rimlight")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
      set_pixel_shader_combo(IN_RIMLIGHT, 1);

      has_rimlight = true;

      // Default exponent is 4, boost is 2, rim mask disabled.
      LVecBase3 rimlight_params(4.0f, 2.0f, 0.0f);
      if ((param = src_mat->get_param("rimlightexponent")) != nullptr) {
        rimlight_params[0] = DCAST(MaterialParamFloat, param)->get_value();
      }
      if ((param = src_mat->get_param("rimlightboost")) != nullptr) {
        rimlight_params[1] = DCAST(MaterialParamFloat, param)->get_value();
      }
      if ((param = src_mat->get_param("rimmask")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
        // Rim lighting mask through phong exponent alpha channel enabled.
        rimlight_params[2] = 1.0f;
      }
      set_input(ShaderInput("rimLightParams", rimlight_params));
    }
  }

#if 1
  if ((param = src_mat->get_param("selfillum")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
    // Self-illum enabled.
    set_pixel_shader_combo(IN_SELFILLUM, 1);

    LVecBase3 selfillum_tint(1.0f, 1.0f, 1.0f);
    if ((param = src_mat->get_param("selfillumtint")) != nullptr) {
      selfillum_tint = DCAST(MaterialParamVector, param)->get_value();
    }
    set_input(ShaderInput("selfIllumTint", selfillum_tint));

    if ((param = src_mat->get_param("selfillummask")) != nullptr) {
      set_pixel_shader_combo(IN_SELFILLUMMASK, 1);
      set_input(ShaderInput("selfIllumMaskTexture", DCAST(MaterialParamTexture, param)->get_value()));
    }
  }
#endif

  if (has_direct_light && (param = src_mat->get_param("halflambert")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
    // Half-lambert diffuse.
    set_spec_constant(IN_HALFLAMBERT, true);
  }

  bool has_envmap = false;

  // If we're using the original source shader, respect the "envmap" material property.
  // Otherwise, always apply an envmap.
  if (!use_orig_source_shader || ((param = src_mat->get_param("envmap")) != nullptr && DCAST(MaterialParamBool, param)->get_value())) {
    Texture *envmap_tex = nullptr;

    const TextureAttrib *ta;
    state->get_attrib_def(ta);
    for (int i = 0; i < ta->get_num_on_stages(); i++) {
      TextureStage *stage = ta->get_on_stage(i);
      const std::string &stage_name = stage->get_name();
      if (stage_name == "envmap") {
        envmap_tex = ta->get_on_texture(stage);
        break;
      }
    }

    if (envmap_tex == nullptr) {
      envmap_tex = ShaderManager::get_global_ptr()->get_default_cube_map();
    }

    if (envmap_tex != nullptr) {
      envmap_tex->set_wrap_u(SamplerState::WM_clamp);
      envmap_tex->set_wrap_v(SamplerState::WM_clamp);
      set_pixel_shader_combo(IN_ENVMAP, 1);
      if ((param = src_mat->get_param("basealphaenvmapmask")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
        set_spec_constant(IN_BASEMAPALPHAENVMAPMASK, true);

      } else if ((param = src_mat->get_param("normalmapalphaenvmapmask")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
        set_spec_constant(IN_NORMALMAPALPHAENVMAPMASK, true);
      }

      LVecBase3 envmap_tint(0.5f, 0.5f, 0.5f);
      if ((param = src_mat->get_param("envmaptint")) != nullptr) {
        envmap_tint = DCAST(MaterialParamVector, param)->get_value();
      }
      set_input(ShaderInput("envMapTint", envmap_tint));

      set_input(ShaderInput("envMapTexture", envmap_tex));

      has_envmap = true;
    }
  }

  if ((has_direct_light || has_ambient_probe || has_envmap || has_rimlight) &&
      (param = src_mat->get_param("bumpmap")) != nullptr) {
    set_pixel_shader_combo(IN_BUMPMAP, 1);
    set_input(ShaderInput("normalTexture", DCAST(MaterialParamTexture, param)->get_value()));
  }

}
