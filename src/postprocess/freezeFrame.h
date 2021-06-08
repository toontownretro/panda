/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freezeFrame.h
 * @author brian
 * @date 2021-06-07
 */

#ifndef FREEZEFRAME_H
#define FREEZEFRAME_H

#include "pandabase.h"
#include "postProcessEffect.h"

class FreezeFrameLayer;

/**
 * Post-processing stage that implements freeze framing.
 */
class EXPCL_PANDA_POSTPROCESS FreezeFrameEffect : public PostProcessEffect {
  DECLARE_CLASS(FreezeFrameEffect, PostProcessEffect);

PUBLISHED:
  FreezeFrameEffect(PostProcess *pp);

  void freeze_frame(double duration);

private:
  // Contains the frame that we are frozen on.
	PT(Texture) _freeze_frame_texture;
	// True if we need to capture a freeze frame.
	bool _take_freeze_frame;
	// When freeze framing, the time at which we will unfreeze.
	double _freeze_frame_until;

  friend class FreezeFrameLayer;
};

#include "freezeFrame.I"

#endif // FREEZEFRAME_H
