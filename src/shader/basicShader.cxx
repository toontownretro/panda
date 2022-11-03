/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file basicShader.cxx
 * @author brian
 * @date 2022-03-16
 */

#include "basicShader.h"
#include "shaderAttrib.h"
#include "internalName.h"
#include "textureAttrib.h"
#include "texture.h"
#include "fogAttrib.h"
#include "fog.h"
#include "alphaTestAttrib.h"
#include "clipPlaneAttrib.h"
#include "geomVertexAnimationSpec.h"
#include "material.h"
#include "materialParamTexture.h"
#include "renderState.h"

TypeHandle BasicShader::_type_handle;

/**
 *
 */
void BasicShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  // Internal names for combos and specialization constants.
  static const CPT_InternalName IN_SKINNING("SKINNING");
  static const CPT_InternalName IN_BASETEXTURE("BASETEXTURE");
  static const CPT_InternalName IN_FOG("FOG");
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");
  static const CPT_InternalName IN_CLIPPING("CLIPPING");
  static const CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");
  static const CPT_InternalName IN_ALPHA_TEST("ALPHA_TEST");
  static const CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");
  static const CPT_InternalName IN_PLANAR_REFLECTION("PLANAR_REFLECTION");
  static const CPT_InternalName IN_BLEND_MODE("BLEND_MODE");

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/basic.vert.sho.pz");
  setup.set_pixel_shader("shaders/basic.frag.sho.pz");

  // Toggle GPU skinning.
  const ShaderAttrib *sha;
  state->get_attrib_def(sha);
  if (sha->has_hardware_skinning()) {
    if (sha->get_num_transforms() > 4) {
      // 8 transforms version.
      setup.set_vertex_shader_combo(IN_SKINNING, 2);
    } else {
      setup.set_vertex_shader_combo(IN_SKINNING, 1);
    }
  }

  if (material == nullptr) {
    const TextureAttrib *ta;
    if (state->get_attrib(ta)) {
      for (int i = 0; i < ta->get_num_on_stages(); ++i) {
        TextureStage *stage = ta->get_on_stage(i);
        if (stage == TextureStage::get_default()) {
          // We have a color texture.
          setup.set_vertex_shader_combo(IN_BASETEXTURE, 1);
          setup.set_pixel_shader_combo(IN_BASETEXTURE, 1);
          setup.set_input(ShaderInput("base_texture_sampler", ta->get_on_texture(stage)));
          break;
        }
      }
    }
  } else {
    MaterialParamBase *p = material->get_param("base_color");
    if (p != nullptr && p->is_of_type(MaterialParamTexture::get_class_type())) {
      setup.set_vertex_shader_combo(IN_BASETEXTURE, 1);
      setup.set_pixel_shader_combo(IN_BASETEXTURE, 1);
      setup.set_input(ShaderInput("base_texture_sampler", DCAST(MaterialParamTexture, p)->get_value()));
    }
  }

  // Check for planar reflection.
  const TextureAttrib *ta;
  if (state->get_attrib(ta)) {
    for (int i = 0; i < ta->get_num_on_stages(); ++i) {
      TextureStage *stage = ta->get_on_stage(i);
      if (stage->get_name() == "reflection") {
        setup.set_vertex_shader_combo(IN_PLANAR_REFLECTION, 1);
        setup.set_pixel_shader_combo(IN_PLANAR_REFLECTION, 1);
        setup.set_input(ShaderInput("reflectionSampler", ta->get_on_texture(stage)));
        break;
      }
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

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      setup.set_pixel_shader_combo(IN_CLIPPING, 1);
      setup.set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }
}
