/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceWater.cxx
 * @author brian
 * @date 2022-07-10
 */

#include "sourceWater.h"
#include "material.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "texture.h"
#include "textureStage.h"
#include "textureStagePool.h"

TypeHandle SourceWater::_type_handle;

/**
 * Returns a dummy four-channel 1x1 white texture.
 */
static Texture *
sw_get_black_texture() {
  static PT(Texture) tex = nullptr;
  if (tex == nullptr) {
    tex = new Texture("sw_black");
    tex->setup_2d_texture(1, 1, Texture::T_unsigned_byte, Texture::F_rgb);
    tex->set_minfilter(SamplerState::FT_nearest);
    tex->set_magfilter(SamplerState::FT_nearest);
    PTA_uchar image;
    image.push_back(0);
    image.push_back(0);
    image.push_back(0);
    tex->set_ram_image(image);
    tex->set_keep_ram_image(false);
  }
  return tex;
}

/**
 * Synthesizes a shader for a given render state.
 */
void SourceWater::
generate_shader(GraphicsStateGuardianBase *gsg,
                const RenderState *state,
                Material *material,
                const GeomVertexAnimationSpec &anim_spec,
                ShaderSetup &setup) {

  setup.set_language(Shader::SL_GLSL);

  setup.set_vertex_shader("shaders/source_water.vert.sho.pz");
  setup.set_pixel_shader("shaders/source_water.frag.sho.pz");

  static TextureStage *lm_stage = TextureStagePool::get_stage(new TextureStage("lightmap"));
  static TextureStage *refl_stage = TextureStagePool::get_stage(new TextureStage("reflection"));
  static TextureStage *refr_stage = TextureStagePool::get_stage(new TextureStage("refraction"));

  const TextureAttrib *ta;
  state->get_attrib_def(ta);

  Texture *lm_tex = ta->get_on_texture(lm_stage);
  nassertv(lm_tex != nullptr);

  Texture *refl_tex = ta->get_on_texture(refl_stage);
  if (refl_tex == nullptr) {
    refl_tex = sw_get_black_texture();
  }
  Texture *refr_tex = ta->get_on_texture(refr_stage);
  if (refr_tex == nullptr) {
    refr_tex = sw_get_black_texture();
  }

  setup.set_input(ShaderInput("lightmapSampler", lm_tex));
  setup.set_input(ShaderInput("reflectionSampler", refl_tex));
  setup.set_input(ShaderInput("refractionSampler", refr_tex));
}
