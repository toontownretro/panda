/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessFinalOutput.cxx
 * @author brian
 * @date 2021-06-08
 */

#include "postProcessFinalOutput.h"
#include "postProcessPass.h"
#include "postProcess.h"

IMPLEMENT_CLASS(PostProcessFinalOutput);

/**
 *
 */
PostProcessFinalOutput::
PostProcessFinalOutput(PostProcess *pp) :
  PostProcessEffect(pp, "final-output-stage") {

  PT(PostProcessPass) layer = new PostProcessPass(pp, "final-output-layer");
  // We are a display region of the output window.
  layer->set_window_layer(true);
  layer->setup();
  // Make sure this is the first display region rendered to the window.
  layer->get_display_region()->set_sort(-1000);

  NodePath quad = layer->get_quad();
  quad.set_shader(
    Shader::load(
      Shader::SL_GLSL,
      "shaders/postprocess/base.vert.glsl",
      "shaders/postprocess/final_output.frag.glsl"
    )
  );
  quad.set_shader_input(ShaderInput("sceneColorSampler", pp->get_output_pipe("scene_color")));

  add_pass(layer);
}
