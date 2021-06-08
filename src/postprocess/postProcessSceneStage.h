/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessSceneStage.h
 * @author brian
 * @date 2021-06-08
 */

#ifndef POSTPROCESSSCENESTAGE_H
#define POSTPROCESSSCENESTAGE_H

#include "pandabase.h"
#include "postProcessEffect.h"

class PostProcessScenePass;

/**
 * Post-processing stage that renders the scene into a set of textures for
 * future stages to operate on.  This should be the first stage added to the
 * pipeline.
 */
class EXPCL_PANDA_POSTPROCESS PostProcessSceneStage : public PostProcessEffect {
  DECLARE_CLASS(PostProcessSceneStage, PostProcessEffect);

PUBLISHED:
  PostProcessSceneStage(PostProcess *pp);

  INLINE PostProcessScenePass *get_scene_pass() const;

private:
  PostProcessScenePass *_scene_pass;
};

#include "postProcessSceneStage.I"

#endif // POSTPROCESSSCENESTAGE_H
