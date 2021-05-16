/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file toneMapping.cxx
 * @author brian
 * @date 2021-05-16
 */

#include "config_postprocess.h"
#include "toneMapping.h"
#include "postProcessPass.h"
#include "postProcess.h"
#include "configVariableEnum.h"

enum ToneMapper {
  TM_aces,
  TM_uncharted_2,
  TM_urchima,
};

std::ostream &
operator << (std::ostream &out, ToneMapper tm) {
  switch (tm) {
  case TM_aces:
    return out << "aces";
  case TM_uncharted_2:
    return out << "uncharted2";
  case TM_urchima:
    return out << "urchima";
  }

  return out << "**invalid ToneMapper (" << (int)tm << ")**";
}

std::istream &
operator >> (std::istream &in, ToneMapper &sm) {
  std::string word;
  in >> word;

  if (cmp_nocase(word, "aces") == 0) {
    sm = TM_aces;
  } else if (cmp_nocase(word, "uncharted2") == 0) {
    sm = TM_uncharted_2;
  } else if (cmp_nocase(word, "urchima") == 0) {
    sm = TM_urchima;

  } else {
    postprocess_cat.error() << "Invalid ToneMapper value: " << word << "\n";
    sm = TM_urchima;
  }

  return in;
}

static ConfigVariableEnum<ToneMapper> tone_mapping_algorithm
("tone-mapping-algorithm", TM_urchima,
 PRC_DESC("Tone-mapping algorithm to use."));

IMPLEMENT_CLASS(ToneMappingEffect);

/**
 *
 */
ToneMappingEffect::
ToneMappingEffect(PostProcess *pp) :
  PostProcessEffect(pp, "tone-mapping")
{
  PT(PostProcessPass) pass = new PostProcessPass(pp, "tone-mapping-apply");

  FrameBufferProperties fbprops = pass->get_framebuffer_properties();
  fbprops.set_rgba_bits(16, 16, 16, 0);
  pass->set_framebuffer_properties(fbprops);

  pass->setup();

  NodePath quad = pass->get_quad();

  Filename pixel_shader;
  switch (tone_mapping_algorithm.get_value()) {
  case TM_aces:
    pixel_shader = "shaders/postprocess/tonemap_aces.frag.glsl";
    break;
  case TM_uncharted_2:
    pixel_shader = "shaders/postprocess/tonemap_uncharted_2.frag.glsl";
    break;
  case TM_urchima:
  default:
    pixel_shader = "shaders/postprocess/tonemap_urchima.frag.glsl";
    break;
  }

  quad.set_shader(
    Shader::load(Shader::SL_GLSL,
      "shaders/postprocess/base.vert.glsl",
      pixel_shader));
  quad.set_shader_input("sceneTexture", pp->get_output_pipe("scene_color"));

  pass->add_color_output();
  pp->push_output_pipe("scene_color", pass->get_color_texture());

  add_pass(pass);
}

Texture *ToneMappingEffect::
get_final_texture() {
  // Need to remove this method.
  return nullptr;
}
