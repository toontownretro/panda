/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessEffect.h
 * @author lachbr
 * @date 2019-07-24
 */

#ifndef POSTPROCESSEFFECT_H
#define POSTPROCESSEFFECT_H

#include "config_postprocess.h"

#include "typedObject.h"
#include "referenceCount.h"
#include "namable.h"
#include "simpleHashMap.h"
#include "pointerTo.h"

class PostProcessPass;
class PostProcess;
class GraphicsOutput;
class Texture;

class EXPCL_PANDA_POSTPROCESS PostProcessEffect : public ReferenceCount, public Namable
{
	DECLARE_CLASS2( PostProcessEffect, ReferenceCount, Namable );

PUBLISHED:
	INLINE PostProcessEffect( PostProcess *pp, const std::string &name = "effect" ) :
		Namable( name ),
		_pp( pp )
	{
	}

	virtual Texture *get_final_texture() = 0;

	void add_pass( PostProcessPass *pass );
	void remove_pass( PostProcessPass *pass );
	PostProcessPass *get_pass( const std::string &name );

	virtual void setup();
	virtual void update();
	virtual void window_event( GraphicsOutput *win );

	virtual void shutdown();

protected:
	PostProcess *_pp;
	SimpleHashMap<std::string, PT( PostProcessPass ), string_hash> _passes;
};

#endif // POSTPROCESSEFFECT_H
