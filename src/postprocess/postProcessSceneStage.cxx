/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessSceneStage.cxx
 * @author brian
 * @date 2021-06-08
 */

#include "postProcessSceneStage.h"
#include "postProcess.h"
#include "postProcessScenePass.h"

IMPLEMENT_CLASS(PostProcessSceneStage);

/**
 *
 */
PostProcessSceneStage::
PostProcessSceneStage(PostProcess *pp) :
  PostProcessEffect(pp, "scene-stage")
{
  PT(PostProcessScenePass) pass = new PostProcessScenePass(pp);
  pass->setup();
  pass->add_color_output();
  pp->push_output_pipe("scene_color", pass->get_color_texture());
  add_pass(pass);
  _scene_pass = pass;
}
