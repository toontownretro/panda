/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthShader.cxx
 * @author lachbr
 * @date 2020-12-16
 */

#include "depthShader.h"
#include "renderState.h"
#include "transparencyAttrib.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "material.h"
#include "materialParamTexture.h"
#include "materialParamColor.h"

TypeHandle DepthShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void DepthShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/depth.vert.glsl");
  set_pixel_shader("shaders/depth.frag.glsl");

  // Do we have transparency?
  add_transparency(state);
  add_alpha_test(state);

  // Hardware skinning?
  add_hardware_skinning(anim_spec);

  // How about clip planes?
  if (add_clip_planes(state)) {
    set_vertex_shader_define("NEED_WORLD_POSITION");
    set_pixel_shader_define("NEED_WORLD_POSITION");
  }

  // Need textures for alpha-tested shadows.

  if (material == nullptr) {
    const TextureAttrib *ta;
    state->get_attrib_def(ta);

    Texture *tex = ta->get_on_texture(TextureStage::get_default());
    if (tex != nullptr) {
      set_pixel_shader_define("BASETEXTURE");
      set_vertex_shader_define("BASETEXTURE");
      set_input(ShaderInput("baseTextureSampler", tex));
    } else {
      set_input(ShaderInput("baseColor", LColor(1, 1, 1, 1)));
    }

  } else {
    MaterialParamBase *param = material->get_param("$basecolor");
    if (param != nullptr) {
      if (param->is_of_type(MaterialParamTexture::get_class_type())) {
        set_pixel_shader_define("BASETEXTURE");
        set_vertex_shader_define("BASETEXTURE");
        set_input(ShaderInput("baseTextureSampler", DCAST(MaterialParamTexture, param)->get_value()));

      } else if (param->is_of_type(MaterialParamColor::get_class_type())) {
        set_input(ShaderInput("baseColor", DCAST(MaterialParamColor, param)->get_value()));
      }

    } else {
      set_input(ShaderInput("baseColor", LColor(1, 1, 1, 1)));
    }
  }

}
