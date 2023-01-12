/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ssao.h
 * @author brian
 */

#ifndef SSAO_H
#define SSAO_H

#include "postProcessEffect.h"

class EXPCL_PANDA_POSTPROCESS SSAO_Effect : public PostProcessEffect
{
	DECLARE_CLASS( SSAO_Effect, PostProcessEffect )

PUBLISHED:
	enum Mode
	{
		M_SSAO,
		M_HBAO
	};

	SSAO_Effect( PostProcess *pp, Mode mode );

	virtual Texture *get_final_texture();

private:
	PT(Texture) _final_texture;
};

#endif // SSAO_H
