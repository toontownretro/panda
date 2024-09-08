/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceSkyShader.cxx
 * @author brian
 * @date 2022-02-15
 */

#include "sourceSkyShader.h"
#include "material.h"
#include "materialParamTexture.h"
#include "materialParamBool.h"
#include "materialParamVector.h"
#include "materialParamMatrix.h"
#include "texture.h"
#include "samplerState.h"
#include "luse.h"
#include "shaderManager.h"

TypeHandle SourceSkyShader::_type_handle;

/**
 * Synthesizes a shader for a given render state.
 */
void SourceSkyShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  static CPT_InternalName IN_COMPRESSED_HDR("COMPRESSED_HDR");

  ShaderManager *mgr = ShaderManager::get_global_ptr();

  setup.set_language(Shader::SL_GLSL);
  setup.set_vertex_shader("shaders/source_sky.vert.sho.pz");
  setup.set_pixel_shader("shaders/source_sky.frag.sho.pz");

  MaterialParamBase *param;

  bool compressed_hdr = false;
  if ((param = material->get_param("compressed_hdr")) != nullptr && DCAST(MaterialParamBool, param)->get_value()) {
    compressed_hdr = true;
  }

  Texture *sky_tex;
  SamplerState sky_sampler;
  if ((param = material->get_param("sky_texture")) != nullptr) {
    sky_tex = DCAST(MaterialParamTexture, param)->get_value();
    sky_sampler = DCAST(MaterialParamTexture, param)->get_sampler_state();

  } else {
    sky_tex = mgr->get_white_texture();
    sky_sampler = sky_tex->get_default_sampler();
  }

  LMatrix4 tex_transform = LMatrix4::ident_mat();
  if ((param = material->get_param("texcoord_transform")) != nullptr) {
    tex_transform = DCAST(MaterialParamMatrix, param)->get_value();
  }

  setup.set_input(ShaderInput("skySampler", sky_tex, sky_sampler));
  setup.set_input(ShaderInput("skyTexTransform", tex_transform));

  LVecBase3 scale(1);

  if (compressed_hdr) {
    setup.set_vertex_shader_combo(IN_COMPRESSED_HDR, 1);
    setup.set_pixel_shader_combo(IN_COMPRESSED_HDR, 1);

    // Stuff for manual bilinear interp of RGBScale texture.
    PN_stdfloat w = sky_tex->get_x_size();
    PN_stdfloat h = sky_tex->get_y_size();
    PN_stdfloat fudge = 0.01f / std::max(w, h);
    setup.set_input(ShaderInput("textureSizeInfo", LVecBase4(0.5f/w-fudge, 0.5f/h-fudge, w, h)));

    scale.set(8.0f, 8.0f, 8.0f);
  }

  setup.set_input(ShaderInput("skyColorScale", scale));

  // Z far and sky face index shader inputs set on sky card nodes
  // themselves.
}
