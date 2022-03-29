/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file csmDepthShader.cxx
 * @author lachbr
 * @date 2020-10-30
 */

#include "csmDepthShader.h"
#include "renderState.h"
#include "transparencyAttrib.h"
#include "lightAttrib.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "cascadeLight.h"
#include "material.h"
#include "materialParamTexture.h"
#include "materialParamColor.h"
#include "clipPlaneAttrib.h"
#include "geomVertexAnimationSpec.h"
#include "transparencyAttrib.h"
#include "alphaTestAttrib.h"

TypeHandle CSMDepthShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void CSMDepthShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  static CPT_InternalName IN_BASETEXTURE("BASETEXTURE");
  static CPT_InternalName IN_HAS_ALPHA("HAS_ALPHA");
  static CPT_InternalName IN_CLIPPING("CLIPPING");
  static CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");
  static CPT_InternalName IN_SKINNING("SKINNING");
  static CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");

  set_language(Shader::SL_GLSL);

  // If the ARB_shader_viewport_layer_array extension is available,
  // we can write to the appropriate cascade layer directly from the
  // vertex shader, eliminating the need for the slow geometry shader.
  //
  // If we don't have it, we have to use the geometry shader.
  bool can_write_gl_layer_from_vertex_shader =
    (gsg->get_supported_shader_capabilities() & ShaderModule::C_viewport_layer_array) != 0;
  nassertv(can_write_gl_layer_from_vertex_shader);
  // TODO: Fallback to geometry shader or texture atlas if not supported.

  set_vertex_shader("shaders/csmdepth.vert.sho.pz");
  //set_geometry_shader("shaders/csmdepth.geom.glsl");
  set_pixel_shader("shaders/csmdepth.frag.sho.pz");

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
    set_pixel_shader_combo(IN_HAS_ALPHA, 1);
    set_spec_constant(IN_ALPHA_TEST_MODE, (int)alpha_mode);
    set_spec_constant(IN_ALPHA_TEST_REF, alpha_ref);
  }

  // Need textures for alpha-tested shadows.

  if (material == nullptr) {
    Texture *tex = nullptr;
    if (has_alpha) {
      const TextureAttrib *texattr;
      state->get_attrib_def(texattr);
      tex = texattr->get_on_texture(TextureStage::get_default());
    }
    if (tex != nullptr) {
      set_vertex_shader_combo(IN_BASETEXTURE, 1);
      set_pixel_shader_combo(IN_BASETEXTURE, 1);
      set_input(ShaderInput("baseTextureSampler", tex));
    } else {
      set_input(ShaderInput("baseColor", LColor(1, 1, 1, 1)));
    }

  } else {
    MaterialParamBase *param = nullptr;
    if (has_alpha) {
      param = material->get_param("base_color");
    }
    if (param != nullptr) {
      if (param->is_of_type(MaterialParamTexture::get_class_type())) {
        set_vertex_shader_combo(IN_BASETEXTURE, 1);
        set_pixel_shader_combo(IN_BASETEXTURE, 1);
        set_input(ShaderInput("baseTextureSampler", DCAST(MaterialParamTexture, param)->get_value()));

      } else if (param->is_of_type(MaterialParamColor::get_class_type())) {
        set_input(ShaderInput("baseColor", DCAST(MaterialParamColor, param)->get_value()));
      }

    } else {
      set_input(ShaderInput("baseColor", LColor(1, 1, 1, 1)));
    }
  }

  if (anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      anim_spec.get_num_transforms() > 0) {
    set_vertex_shader_combo(IN_SKINNING, 1);
  }

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      set_pixel_shader_combo(IN_CLIPPING, 1);
      set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }

  // Find the number of cascades
  const LightAttrib *lattr;
  state->get_attrib_def(lattr);

  // Have to call this to sort the lights if they haven't been yet...
  // Why can't check_sorted() be public?
  lattr->get_num_non_ambient_lights();

  // It is set up so that the only light we have is the light we are rendering
  // shadows for, so just expect that.
  CascadeLight *clight = DCAST(CascadeLight, lattr->get_on_light(0).node());

  // Instance the geometry to each cascade.
  set_instance_count(clight->get_num_cascades());
}
