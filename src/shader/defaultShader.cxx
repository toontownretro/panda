/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file defaultShader.cxx
 * @author lachbr
 * @date 2020-10-18
 */

#include "defaultShader.h"
#include "shaderAttrib.h"
#include "alphaTestAttrib.h"
#include "renderState.h"
#include "luse.h"

TypeHandle DefaultShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void DefaultShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                const ShaderParamAttrib *params,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  std::ostringstream vss;
  vss <<
    "#version 330\n"
    "in vec4 p3d_Vertex;\n"
    "in vec4 p3d_Color;\n"
    "in vec2 texcoord;\n"
    "uniform mat4 p3d_ModelViewProjectionMatrix;\n"
    "uniform vec4 p3d_ColorScale;\n"
    "out vec4 l_color;\n"
    "out vec2 l_texcoord;\n"
    "void main() {\n"
    "  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;\n"
    "  l_texcoord = texcoord;\n"
    "  l_color = p3d_Color * p3d_ColorScale;\n"
    "}\n";

  set_vertex_shader_source(vss.str());

  const AlphaTestAttrib *alpha_test;
	state->get_attrib_def(alpha_test);
  bool do_alpha_test = (alpha_test->get_mode() != RenderAttrib::M_none &&
		                    alpha_test->get_mode() != RenderAttrib::M_always);

  std::ostringstream pss;
  pss <<
    "#version 330\n"
    "in vec2 l_texcoord;\n"
    "in vec4 l_color;\n"
    "uniform vec4 p3d_TexAlphaOnly;\n"
    "uniform sampler2D p3d_Texture0;\n"
    //"uniform vec4 bruh;\n"
    "out vec4 p3d_FragColor;\n"
    "void main() {\n"
    "  p3d_FragColor = texture(p3d_Texture0, l_texcoord);\n"
    "  p3d_FragColor += p3d_TexAlphaOnly;\n"
    "  p3d_FragColor *= l_color;\n";
  if (do_alpha_test) {
    pss << "  if (p3d_FragColor.a < 0.5) discard;\n";
  }
  pss << "}\n";

  set_pixel_shader_source(pss.str());

  set_flags(ShaderAttrib::F_subsume_alpha_test);

  //set_input(ShaderInput("bruh", LVector4(0, 0, 1, 1)));
}
