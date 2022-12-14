/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fxaa.h
 * @author lachbr
 * @date 2019-12-07
 */

#ifndef FXAA_H
#define FXAA_H

#include "pandabase.h"
#include "postProcessEffect.h"

class Texture;
class PostProcess;

/**
 *
 */
class EXPCL_PANDA_POSTPROCESS FXAA_Effect : public PostProcessEffect {
	DECLARE_CLASS(FXAA_Effect, PostProcessEffect);

PUBLISHED:
	FXAA_Effect(PostProcess *pp);
};

#endif // FXAA_H
