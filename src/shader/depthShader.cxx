/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthShader.cxx
 * @author brian
 * @date 2020-12-16
 */

#include "depthShader.h"
#include "renderState.h"
#include "shaderAttrib.h"
#include "transparencyAttrib.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "material.h"
#include "materialParamTexture.h"
#include "materialParamColor.h"
#include "alphaTestAttrib.h"
#include "clipPlaneAttrib.h"
#include "geomVertexAnimationSpec.h"

TypeHandle DepthShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void DepthShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  static CPT_InternalName IN_BASETEXTURE("BASETEXTURE");
  static CPT_InternalName IN_HAS_ALPHA("HAS_ALPHA");
  static CPT_InternalName IN_CLIPPING("CLIPPING");
  static CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");
  static CPT_InternalName IN_SKINNING("SKINNING");
  static CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/depth.vert.sho.pz");
  setup.set_pixel_shader("shaders/depth.frag.sho.pz");

  // Check if alpha is enabled (indicating we should do alpha cutouts for
  // shadows).
  // If alpha testing is enabled on the state, the shader will perform the
  // specified alpha test.  If transparency is enabled, the shader will discard
  // pixels with alpha values < 0.5.
  bool has_alpha = false;
  PN_stdfloat alpha_ref = 0.5f;
  AlphaTestAttrib::PandaCompareFunc alpha_mode = AlphaTestAttrib::M_greater_equal;
  const TransparencyAttrib *ta;
  const AlphaTestAttrib *ata;
  if (state->get_attrib(ata) && ata->get_mode() != AlphaTestAttrib::M_always &&
      ata->get_mode() != AlphaTestAttrib::M_none) {
    has_alpha = true;
    alpha_ref = ata->get_reference_alpha();
    alpha_mode = ata->get_mode();

  } else if (state->get_attrib(ta) && ta->get_mode() != TransparencyAttrib::M_none) {
    has_alpha = true;
  }

  if (has_alpha) {
    setup.set_pixel_shader_combo(IN_HAS_ALPHA, 1);
    setup.set_spec_constant(IN_ALPHA_TEST_MODE, (int)alpha_mode);
    setup.set_spec_constant(IN_ALPHA_TEST_REF, alpha_ref);
  }

  // Need textures for alpha-tested shadows.

  if (material == nullptr) {
    Texture *tex = nullptr;
    SamplerState samp;
    if (has_alpha) {
      const TextureAttrib *texattr;
      state->get_attrib_def(texattr);
      tex = texattr->get_on_texture(TextureStage::get_default());
      samp = texattr->get_on_sampler(TextureStage::get_default());
    }
    if (tex != nullptr) {
      setup.set_vertex_shader_combo(IN_BASETEXTURE, 1);
      setup.set_pixel_shader_combo(IN_BASETEXTURE, 1);
      setup.set_input(ShaderInput("baseTextureSampler", tex, samp));
    } else {
      setup.set_input(ShaderInput("baseColor", LColor(1, 1, 1, 1)));
    }

  } else {
    MaterialParamBase *param = nullptr;
    if (has_alpha) {
      param = material->get_param("base_color");
    }
    if (param != nullptr) {
      if (param->is_of_type(MaterialParamTexture::get_class_type())) {
        setup.set_vertex_shader_combo(IN_BASETEXTURE, 1);
        setup.set_pixel_shader_combo(IN_BASETEXTURE, 1);
        MaterialParamTexture *tex_p = DCAST(MaterialParamTexture, param);
        setup.set_input(ShaderInput("baseTextureSampler", tex_p->get_value(), tex_p->get_sampler_state()));

      } else if (param->is_of_type(MaterialParamColor::get_class_type())) {
        setup.set_input(ShaderInput("baseColor", DCAST(MaterialParamColor, param)->get_value()));
      }

    } else {
      setup.set_input(ShaderInput("baseColor", LColor(1, 1, 1, 1)));
    }
  }

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

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      setup.set_pixel_shader_combo(IN_CLIPPING, 1);
      setup.set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }

}
