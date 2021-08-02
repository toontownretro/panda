/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file skyBoxShader.cxx
 * @author brian
 * @date 2021-07-13
 */

#include "skyBoxShader.h"
#include "shaderManager.h"

TypeHandle SkyBoxShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void SkyBoxShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material_base,
                const GeomVertexAnimationSpec &anim_spec) {

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shaders/skybox.vert.glsl");
  set_pixel_shader("shaders/skybox.frag.glsl");

  Texture *sky_cube = nullptr;
  if (material_base != nullptr) {
    SkyBoxMaterial *material;
    DCAST_INTO_V(material, material_base);
    sky_cube = material->get_sky_cube_map();
  }

  if (sky_cube == nullptr) {
    sky_cube = ShaderManager::get_global_ptr()->get_default_cube_map();
  }

  set_input(ShaderInput("skyboxSampler", sky_cube));
}
