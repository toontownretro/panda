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
#include "paramAttrib.h"
#include "keyValues.h"
#include "lightLensNode.h"
#include "light.h"

TypeHandle VertexLitShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void VertexLitShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const ParamAttrib *params,
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

  bool put_shadowed_light = false;
  bool put_shadowed_point_light = false;
  bool put_shadowed_spotlight = false;

  // Break out the lights by type.
  const LightAttrib *la;
  state->get_attrib_def(la);
  size_t num_lights = la->get_num_non_ambient_lights();
  size_t num_ambient_lights = la->get_num_on_lights() - num_lights;
  if (num_ambient_lights != 0) {
    set_pixel_shader_define("AMBIENT_LIGHT");
  }
  if (num_lights > 0) {
    need_world_vec = true;
    need_world_normal = true;

    set_pixel_shader_define("LIGHTING");
    set_pixel_shader_define("NUM_LIGHTS", num_lights);
    set_vertex_shader_define("NUM_LIGHTS", num_lights);

    for (size_t i = 0; i < num_lights; i++) {
      if (put_shadowed_light && put_shadowed_point_light &&
          put_shadowed_spotlight) {
        break;
      }

      NodePath light = la->get_on_light(i);
      Light *light_obj = light.node()->as_light();
      LightLensNode *light_lens = DCAST(LightLensNode, light.node());
      if (light_lens->is_shadow_caster() &&
          light_obj->get_light_type() != Light::LT_directional) {

        if (!put_shadowed_light) {
          set_pixel_shader_define("HAS_SHADOWED_LIGHT");
          set_vertex_shader_define("HAS_SHADOWED_LIGHT");
          need_eye_position = true;
          put_shadowed_light = true;
        }

        switch (light_obj->get_light_type()) {
        case Light::LT_point:
          if (!put_shadowed_point_light) {
            set_pixel_shader_define("HAS_SHADOWED_POINT_LIGHT");
            set_vertex_shader_define("HAS_SHADOWED_POINT_LIGHT");
            put_shadowed_point_light = true;
          }
          break;
        case Light::LT_spot:
          if (!put_shadowed_spotlight) {
            set_pixel_shader_define("HAS_SHADOWED_SPOTLIGHT");
            set_vertex_shader_define("HAS_SHADOWED_SPOTLIGHT");
            put_shadowed_spotlight = true;
          }
          break;
        default:
          break;
        }
      }
    }
  }

  // Are we self-illuminating?
  int selfillum_idx = params->find_param("selfillum");
  if (selfillum_idx != -1) {
    bool selfillum = params->get_param_value_bool(selfillum_idx);
    if (selfillum) {
      // Selfillum is enabled.
      set_pixel_shader_define("SELFILLUM");

      // Now figure out the tint value.
      LVecBase3f tint(1.0);
      int tint_idx = params->find_param("selfillum_tint");
      if (tint_idx != -1) {
        // Got an explicit tint value.
        tint = params->get_param_value_3f(tint_idx);
      }
      set_input(ShaderInput("selfillumTint", tint));
    }
  }

  // Rimlight?
  int rimlight_idx = params->find_param("rimlight");
  if (rimlight_idx != -1) {
    bool rimlight = params->get_param_value_bool(rimlight_idx);

    if (rimlight && ConfigVariableBool("mat_rimlight", true)) {
      float boost = 1.0f;
      float exponent = 4.0f;

      int boost_idx = params->find_param("rimlight_boost");
      if (boost_idx != -1) {
        boost = params->get_param_value_float(boost_idx);
      }

      int exponent_idx = params->find_param("rimlight_exponent");
      if (exponent_idx != -1) {
        exponent = params->get_param_value_float(exponent_idx);
      }

      set_pixel_shader_define("RIMLIGHT");
      set_input(ShaderInput("rimlightParams", LVector2(boost, exponent)));
    }
  }

  // Half-lambert?
  int halflambert_idx = params->find_param("half_lambert");
  if (halflambert_idx != -1) {
    bool half_lambert = params->get_param_value_bool(halflambert_idx);
    if (half_lambert) {
      set_pixel_shader_define("HALFLAMBERT");
    }
  }

  bool got_arme_texture = false;
  bool got_base_color_texture = false;
  bool got_envmap_texture = false;

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
      got_base_color_texture = true;

    } else if (stage_name == "arme") {
      set_pixel_shader_define("ARME");
      set_input(ShaderInput("armeSampler", ta->get_on_texture(stage)));
      got_arme_texture = true;

    } else if (stage_name == "reflection") {
      set_pixel_shader_define("PLANAR_REFLECTION");
      set_vertex_shader_define("PLANAR_REFLECTION");
      set_input(ShaderInput("reflectionSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "normal") {
      set_pixel_shader_define("BUMPMAP");
      set_input(ShaderInput("bumpSampler", ta->get_on_texture(stage)));

    } else if (stage_name == "envmap") {
      set_pixel_shader_define("ENVMAP");
      set_input(ShaderInput("envmapSampler", ta->get_on_texture(stage)));
      got_envmap_texture = true;

    } else if (stage_name == "lightwarp") {
      set_pixel_shader_define("LIGHTWARP");
      set_input(ShaderInput("lightwarpSampler", ta->get_on_texture(stage)));
    }
  }

  set_vertex_shader_define("NUM_TEXTURES", num_stages);

  if (!got_base_color_texture) {
    // Check for an explicit base color value from material parameters.
    int base_color_idx = params->find_param("base_color");
    if (base_color_idx != -1) {
      LVector4f base_color = params->get_param_value_4f(base_color_idx);
      set_pixel_shader_define("BASECOLOR");
      set_input(ShaderInput("baseColor", base_color));
    }
  }

  if (!got_arme_texture) {
    // Check for explicit ARME values via material parameters.
    float ao = 1.0f;
    float roughness = 1.0f;
    float metalness = 0.0f;
    float emission = 0.0f;

    int ao_idx = params->find_param("ao");
    if (ao_idx != -1) {
      ao = params->get_param_value_float(ao_idx);
    }

    int roughness_idx = params->find_param("roughness");
    if (roughness_idx != -1) {
      roughness = params->get_param_value_float(roughness_idx);
    }

    int metalness_idx = params->find_param("metalness");
    if (metalness_idx != -1) {
      metalness = params->get_param_value_float(metalness_idx);
    }

    int emission_idx = params->find_param("emission");
    if (emission_idx != -1) {
      emission = params->get_param_value_float(emission_idx);
    }

    set_input(ShaderInput("armeParams", LVector4f(ao, roughness, metalness, emission)));
  }

  if (got_envmap_texture) {
    // Check for a tint parameter.
    LVector3f envmap_tint(1.0f);

    int envmap_tint_idx = params->find_param("envmap_tint");
    if (envmap_tint_idx != -1) {
      envmap_tint = params->get_param_value_3f(envmap_tint_idx);
    }

    set_input(ShaderInput("envmapTint", envmap_tint));
  }

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
