/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessFinalOutput.h
 * @author brian
 * @date 2021-06-08
 */

#ifndef POSTPROCESSFINALOUTPUT_H
#define POSTPROCESSFINALOUTPUT_H

#include "pandabase.h"
#include "postProcessEffect.h"

/**
 * Post-processing stage that presents the final post-processed scene to the
 * main window.
 */
class EXPCL_PANDA_POSTPROCESS PostProcessFinalOutput : public PostProcessEffect {
  DECLARE_CLASS(PostProcessFinalOutput, PostProcessEffect);

PUBLISHED:
  PostProcessFinalOutput(PostProcess *pp);
};

#include "postProcessFinalOutput.I"

#endif // POSTPROCESSFINALOUTPUT_H
