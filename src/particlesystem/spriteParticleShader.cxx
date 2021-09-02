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

TypeHandle SpriteParticleShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void SpriteParticleShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/spriteParticle.vert.glsl");
  set_geometry_shader("shaders/spriteParticle.geom.glsl");
  set_pixel_shader("shaders/spriteParticle.frag.glsl");

  add_alpha_test(state);
  add_fog(state);
  add_clip_planes(state);

  const RenderModeAttrib *rma;
  state->get_attrib_def(rma);

  PN_stdfloat x_size, y_size;

  // First use the thickness that the RenderModeAttrib specifies,
  // then modulate it with the sizes specified in the material.
  x_size = y_size = rma->get_thickness();

  if (material != nullptr) {
    MaterialParamFloat *x_size_p = (MaterialParamFloat *)material->get_param("x_size");
    if (x_size_p != nullptr) {
      x_size *= x_size_p->get_value();
    }

    MaterialParamFloat *y_size_p = (MaterialParamFloat *)material->get_param("y_size");
    if (y_size_p != nullptr) {
      y_size *= y_size_p->get_value();
    }
  }

  set_input(ShaderInput("sprite_size", LVecBase2(x_size, y_size)));

  // Now get the texture.
  MaterialParamTexture *tex_p = nullptr;
  if (material != nullptr) {
    tex_p = (MaterialParamTexture *)material->get_param("base_texture");
  }

  if (tex_p != nullptr) {
    // Use the texture specified in the material.
    set_pixel_shader_define("BASETEXTURE");
    set_input(ShaderInput("baseTextureSampler", tex_p->get_value()));

  } else {
    // No texture in material, so use the first one from the TextureAttrib.
    const TextureAttrib *ta;
    state->get_attrib_def(ta);
    if (ta->get_num_on_stages() > 0) {
      set_geometry_shader_define("NUM_TEXTURES", ta->get_num_on_stages());

      for (int i = 0; i < ta->get_num_on_stages(); i++) {
        TextureStage *stage = ta->get_on_stage(i);
        if (stage == TextureStage::get_default()) {
          set_geometry_shader_define("BASETEXTURE_INDEX", i);
          set_pixel_shader_define("BASETEXTURE");
          Texture *tex = ta->get_on_texture(stage);
          set_input(ShaderInput("baseTextureSampler", tex));
          break;
        }
      }
    }
  }
}
