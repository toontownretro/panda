/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bloom.h
 * @author lachbr
 * @date 2019-07-23
 */

#ifndef BLOOM_H
#define BLOOM_H

#include "postProcessEffect.h"

class Texture;
class PostProcess;

class EXPCL_PANDA_POSTPROCESS BloomEffect : public PostProcessEffect
{
	DECLARE_CLASS( BloomEffect, PostProcessEffect );

PUBLISHED:
	BloomEffect( PostProcess *pp );

	virtual Texture *get_final_texture();
};

#endif
