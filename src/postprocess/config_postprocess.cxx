/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_postprocess.cxx
 * @author brian
 * @date 2020-09-20
 */

#include "config_postprocess.h"

#include "bloom.h"
#include "fxaa.h"
#include "hdr.h"
#include "postProcess.h"
#include "postProcessEffect.h"
#include "postProcessPass.h"
#include "ssao.h"
#include "toneMapping.h"
#include "freezeFrame.h"
#include "postProcessSceneStage.h"
#include "postProcessScenePass.h"
#include "postProcessFinalOutput.h"
#include "motionBlur.h"

NotifyCategoryDef(postprocess, "")

ConfigureDef(config_postprocess)

ConfigureFn(config_postprocess) {
  init_libpostprocess();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpostprocess() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  PostProcess::init_type();
  PostProcessPass::init_type();
  PostProcessEffect::init_type();

  BloomEffect::init_type();
  FreezeFrameEffect::init_type();
  FXAA_Effect::init_type();
  HDRPass::init_type();
  HDREffect::init_type();
  PostProcessFinalOutput::init_type();
  PostProcessSceneStage::init_type();
  PostProcessScenePass::init_type();
  SSAO_Effect::init_type();
  ToneMappingEffect::init_type();
  MotionBlur::init_type();
}
