/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteParticleShader.cxx
 * @author brian
 * @date 2021-09-01
 */

#include "spriteParticleShader.h"
#include "materialParamFloat.h"
#include "renderModeAttrib.h"
#include "renderState.h"
#include "materialParamTexture.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "texture.h"
#include "clipPlaneAttrib.h"
#include "fogAttrib.h"
#include "fog.h"
#include "alphaTestAttrib.h"
#include "materialParamBool.h"
#include "shaderAttrib.h"

TypeHandle SpriteParticleShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void SpriteParticleShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  // Internal names for combos and specialization constants.
  static const CPT_InternalName IN_BASETEXTURE("BASETEXTURE");
  static const CPT_InternalName IN_FOG("FOG");
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");
  static const CPT_InternalName IN_CLIPPING("CLIPPING");
  static const CPT_InternalName IN_NUM_CLIP_PLANES("NUM_CLIP_PLANES");
  static const CPT_InternalName IN_ALPHA_TEST("ALPHA_TEST");
  static const CPT_InternalName IN_ALPHA_TEST_MODE("ALPHA_TEST_MODE");
  static const CPT_InternalName IN_ALPHA_TEST_REF("ALPHA_TEST_REF");
  static const CPT_InternalName IN_BILLBOARD_MODE("BILLBOARD_MODE");

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/spriteParticle.vert.sho.pz");
  set_geometry_shader("shaders/spriteParticle.geom.sho.pz");
  set_pixel_shader("shaders/spriteParticle.frag.sho.pz");

  const RenderModeAttrib *rma;
  state->get_attrib_def(rma);

  PN_stdfloat x_size, y_size;

  // First use the thickness that the RenderModeAttrib specifies,
  // then modulate it with the sizes specified in the material.
  x_size = y_size = rma->get_thickness();

  // 0 is point-eye, 1 is point-world.
  int billboard = 0;

  if (material != nullptr) {
    MaterialParamFloat *x_size_p = (MaterialParamFloat *)material->get_param("x_size");
    if (x_size_p != nullptr) {
      x_size *= x_size_p->get_value();
    }

    MaterialParamFloat *y_size_p = (MaterialParamFloat *)material->get_param("y_size");
    if (y_size_p != nullptr) {
      y_size *= y_size_p->get_value();
    }

    MaterialParamBool *point_world_p = (MaterialParamBool *)material->get_param("point_world");
    if (point_world_p != nullptr) {
      billboard = (int)point_world_p->get_value();
    }
  }

  // Bad hack to specify billboard mode through scene graph ShaderAttrib.
  const ShaderAttrib *sha;
  state->get_attrib_def(sha);
  if (sha->has_shader_input(IN_BILLBOARD_MODE)) {
    LVecBase4 value;
    value = sha->get_shader_input_vector(IN_BILLBOARD_MODE);
    billboard = (int)value[0];
  }

  set_geometry_shader_combo(IN_BILLBOARD_MODE, billboard);

  set_input(ShaderInput("sprite_size", LVecBase2(x_size, y_size)));

  // Now get the texture.
  MaterialParamTexture *tex_p = nullptr;
  if (material != nullptr) {
    tex_p = (MaterialParamTexture *)material->get_param("base_texture");
  }

  if (tex_p != nullptr) {
    // Use the texture specified in the material.
    set_pixel_shader_combo(IN_BASETEXTURE, 1);
    set_input(ShaderInput("baseTextureSampler", tex_p->get_value()));

  } else {
    // No texture in material, so use the first one from the TextureAttrib.
    const TextureAttrib *ta;
    state->get_attrib_def(ta);
    if (ta->get_num_on_stages() > 0) {
      for (int i = 0; i < ta->get_num_on_stages(); i++) {
        TextureStage *stage = ta->get_on_stage(i);
        if (stage == TextureStage::get_default()) {
          set_pixel_shader_combo(IN_BASETEXTURE, 1);
          Texture *tex = ta->get_on_texture(stage);
          set_input(ShaderInput("baseTextureSampler", tex));
          break;
        }
      }
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

  const ClipPlaneAttrib *cpa;
  if (state->get_attrib(cpa)) {
    if (cpa->get_num_on_planes() > 0) {
      set_pixel_shader_combo(IN_CLIPPING, 1);
      set_spec_constant(IN_NUM_CLIP_PLANES, cpa->get_num_on_planes());
    }
  }
}
