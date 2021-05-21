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
                Material *material_base,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/vertexLitGeneric_PBR.vert.glsl");
  set_pixel_shader("shaders/vertexLitGeneric_PBR.frag.glsl");

  StandardMaterial *material = DCAST(StandardMaterial, material_base);

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
  if (material->get_emission_enabled()) {
    // Selfillum is enabled.
    set_pixel_shader_define("SELFILLUM");
    set_input(ShaderInput("selfillumTint", material->get_emission_tint()));
  }

  // Rimlight?
  if (material->get_rim_light() && ConfigVariableBool("mat_rimlight", true)) {
    float boost = material->get_rim_light_boost();
    float exponent = material->get_rim_light_exponent();

    set_pixel_shader_define("RIMLIGHT");
    set_input(ShaderInput("rimlightParams", LVector2(boost, exponent)));
  }

  // Half-lambert?
  if (material->get_half_lambert()) {
    set_pixel_shader_define("HALFLAMBERT");
  }

  // The material might want to use the cubemap selected from the environment
  // or a custom cubemap.
  bool env_cubemap = material->get_env_cubemap();
  Texture *cubemap_tex = nullptr;

  // Find the textures in use.
  const TextureAttrib *ta;
  state->get_attrib_def(ta);
  int num_stages = ta->get_num_on_stages();
  for (int i = 0; i < num_stages; i++) {
    TextureStage *stage = ta->get_on_stage(i);
    const std::string &stage_name = stage->get_name();

    if (stage_name == "reflection") {
      set_pixel_shader_define("PLANAR_REFLECTION");
      set_vertex_shader_define("PLANAR_REFLECTION");
      set_input(ShaderInput("reflectionSampler", ta->get_on_texture(stage)));

    } else if (env_cubemap && stage_name == "envmap") {
      cubemap_tex = ta->get_on_texture(stage);
    }
  }

  Texture *base_tex = material->get_base_texture();
  if (base_tex != nullptr) {
    set_pixel_shader_define("BASETEXTURE");
    set_input(ShaderInput("baseTextureSampler", base_tex));

  } else {
    set_pixel_shader_define("BASECOLOR");
    set_input(ShaderInput("baseColor", material->get_base_color()));
  }

  Texture *normal_tex = material->get_normal_texture();
  if (normal_tex != nullptr) {
    set_pixel_shader_define("BUMPMAP");
    set_input(ShaderInput("bumpSampler", normal_tex));
  }

  Texture *arme_tex = material->get_arme_texture();
  if (arme_tex != nullptr) {
    set_pixel_shader_define("ARME");
    set_input(ShaderInput("armeSampler", arme_tex));

  } else {
    float ao = 1.0f;
    float roughness = material->get_roughness();
    float metalness = material->get_metalness();
    float emission = material->get_emission();
    set_input(ShaderInput("armeParams", LVector4f(ao, roughness, metalness, emission)));
  }

  Texture *spec_tex = material->get_specular_texture();
  if (spec_tex != nullptr) {
    set_pixel_shader_define("SPECULAR_MAP");
    set_input(ShaderInput("specularSampler", spec_tex));
  }

  Texture *lw_tex = material->get_lightwarp_texture();
  if (lw_tex != nullptr) {
    set_pixel_shader_define("LIGHTWARP");
    set_input(ShaderInput("lightwarpSampler", lw_tex));
  }

  if (!env_cubemap) {
    cubemap_tex = material->get_envmap_texture();
  }

  if (cubemap_tex) {
    set_pixel_shader_define("ENVMAP");
    set_input(ShaderInput("envmapSampler", cubemap_tex));

    // Check for a tint parameter.
    LVector3f envmap_tint(1.0f);

    //int envmap_tint_idx = params->find_param("envmap_tint");
    //if (envmap_tint_idx != -1) {
    //  envmap_tint = params->get_param_value_3f(envmap_tint_idx);
    //}

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
