/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file twoTextureShader.cxx
 * @author brian
 * @date 2022-03-22
 */

#include "twoTextureShader.h"
#include "fogAttrib.h"
#include "fog.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "texture.h"
#include "material.h"
#include "materialParamTexture.h"
#include "materialParamMatrix.h"
#include "materialParamVector.h"

TypeHandle TwoTextureShader::_type_handle;

/**
 * Returns a dummy four-channel 1x1 white texture.
 */
static Texture *
get_white_texture() {
  static PT(Texture) tex = nullptr;
  if (tex == nullptr) {
    tex = new Texture("white");
    tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgba);
    tex->set_minfilter(SamplerState::FT_nearest);
    tex->set_magfilter(SamplerState::FT_nearest);
    PTA_uchar image;
    image.push_back(255);
    image.push_back(255);
    image.push_back(255);
    image.push_back(255);
    tex->set_ram_image(image);
  }
  return tex;
}

/**
 * Synthesizes a shader for a given render state.
 */
void TwoTextureShader::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec) {

  // Combo names.
  static const CPT_InternalName IN_FOG("FOG");
  static const CPT_InternalName IN_LIGHTMAP("LIGHTMAP");
  static const CPT_InternalName IN_SKINNING("SKINNING");

  // Specialization constant names.
  static const CPT_InternalName IN_FOG_MODE("FOG_MODE");

  set_language(Shader::SL_GLSL);

  set_vertex_shader("shadersnew/two_texture.vert.sho.pz");
  set_pixel_shader("shadersnew/two_texture.frag.sho.pz");

  // Hardware skinning?
  if (anim_spec.get_animation_type() == GeomEnums::AT_hardware &&
      anim_spec.get_num_transforms() > 0) {
    set_vertex_shader_combo(IN_SKINNING, 1);
  }

  const FogAttrib *fa;
  if (state->get_attrib(fa)) {
    Fog *fog = fa->get_fog();
    if (fog != nullptr) {
      set_pixel_shader_combo(IN_FOG, 1);
      set_spec_constant(IN_FOG_MODE, (int)fog->get_mode());
    }
  }

  MaterialParamBase *param;

  if ((param = material->get_param("base_color")) != nullptr) {
    set_input(ShaderInput("baseTexture", DCAST(MaterialParamTexture, param)->get_value()));
  } else {
    set_input(ShaderInput("baseTexture", get_white_texture()));
  }
  if ((param = material->get_param("basetexturetransform")) != nullptr) {
    set_input(ShaderInput("baseTextureTransform", DCAST(MaterialParamMatrix, param)->get_value()));
  } else {
    set_input(ShaderInput("baseTextureTransform", LMatrix4::ident_mat()));
  }

  if ((param = material->get_param("texture2")) != nullptr) {
    set_input(ShaderInput("baseTexture2", DCAST(MaterialParamTexture, param)->get_value()));
  } else {
    set_input(ShaderInput("baseTexture2", get_white_texture()));
  }
  if ((param = material->get_param("texture2transform")) != nullptr) {
    set_input(ShaderInput("baseTexture2Transform", DCAST(MaterialParamMatrix, param)->get_value()));
  } else {
    set_input(ShaderInput("baseTexture2Transform", LMatrix4::ident_mat()));
  }

  LVecBase4 scroll(0.0f);
  if ((param = material->get_param("basetexturescroll")) != nullptr) {
    LVecBase2 one_scroll = DCAST(MaterialParamVector, param)->get_value().get_xy();
    scroll[0] = one_scroll[0];
    scroll[1] = one_scroll[1];
  }
  if ((param = material->get_param("texture2scroll")) != nullptr) {
    LVecBase2 two_scroll = DCAST(MaterialParamVector, param)->get_value().get_xy();
    scroll[2] = two_scroll[0];
    scroll[3] = two_scroll[1];
  }
  set_input(ShaderInput("textureScroll", scroll));

  LVecBase3 sine_x(0.0f, 0.0f, 1.0f);
  LVecBase3 sine_y(0.0f, 0.0f, 1.0f);
  if ((param = material->get_param("basetexturesinex")) != nullptr) {
    sine_x = DCAST(MaterialParamVector, param)->get_value();
  }
  if ((param = material->get_param("basetexturesiney")) != nullptr) {
    sine_y = DCAST(MaterialParamVector, param)->get_value();
  }
  set_input(ShaderInput("sineXParams", sine_x));
  set_input(ShaderInput("sineYParams", sine_y));

  const TextureAttrib *tattr;
  state->get_attrib_def(tattr);
  for (int i = 0; i < tattr->get_num_on_stages(); ++i) {
    TextureStage *stage = tattr->get_on_stage(i);
    const std::string &name = stage->get_name();

    if (name == "lightmap") {
      set_input(ShaderInput("lightmapTexture", tattr->get_on_texture(stage)));
      set_vertex_shader_combo(IN_LIGHTMAP, 1);
      set_pixel_shader_combo(IN_LIGHTMAP, 1);
      break;
    }
  }
}
