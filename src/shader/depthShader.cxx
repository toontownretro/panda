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

TypeHandle DepthShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void DepthShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const ShaderParamAttrib *params,
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
  const TextureAttrib *ta;
  state->get_attrib_def(ta);
  for (int i = 0; i < ta->get_num_on_stages(); i++) {
    TextureStage *stage = ta->get_on_stage(i);
    if (stage == TextureStage::get_default() ||
        stage->get_name() == "albedo") {
      set_pixel_shader_define("BASETEXTURE");
      set_input(ShaderInput("baseTextureSampler", ta->get_on_texture(stage)));
    }
  }
}
