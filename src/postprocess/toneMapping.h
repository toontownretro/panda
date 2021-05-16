/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file toneMapping.h
 * @author brian
 * @date 2021-05-16
 */

#ifndef TONEMAPPING_H
#define TONEMAPPING_H

#include "pandabase.h"
#include "postProcessEffect.h"

/**
 * Post-process effect/stage that performs tone-mapping on the scene using an
 * algorithm of choice.
 */
class EXPCL_PANDA_POSTPROCESS ToneMappingEffect : public PostProcessEffect {
  DECLARE_CLASS(ToneMappingEffect, PostProcessEffect);

PUBLISHED:
  ToneMappingEffect(PostProcess *pp);

  virtual Texture *get_final_texture() override;
};

#include "toneMapping.I"

#endif // TONEMAPPING_H
