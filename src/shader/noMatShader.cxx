/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file noMatShader.cxx
 * @author lachbr
 * @date 2021-03-22
 */

#include "noMatShader.h"
#include "renderState.h"
#include "textureAttrib.h"

TypeHandle NoMatShader::_type_handle;

/**
 *
 */
void NoMatShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/unlitNoMat.vert.sho");
  setup.set_pixel_shader("shaders/unlitNoMat.frag.sho");

  add_alpha_test(state);
  add_aux_attachments(state);

  const TextureAttrib *ta;
  state->get_attrib_def(ta);
  if (ta->get_num_on_stages() > 0) {
    set_vertex_shader_define("HAS_TEXTURE");
    set_pixel_shader_define("HAS_TEXTURE");
  }
}
