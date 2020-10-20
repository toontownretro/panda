/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessScenePass.h
 * @author lachbr
 * @date 2019-07-24
 */

#ifndef POSTPROCESSSCENEPASS_H
#define POSTPROCESSSCENEPASS_H

#include "postProcessPass.h"

#include "renderState.h"

class PostProcess;

class EXPCL_PANDA_POSTPROCESS PostProcessScenePass : public PostProcessPass
{
PUBLISHED:
	PostProcessScenePass( PostProcess *pp );
	virtual void setup_region();
	virtual void setup_camera();
	virtual void setup_quad();
	virtual bool setup_buffer();
	virtual void add_aux_output( int n );

	void setup_scene_camera( int i, int sort = 0 );

	void set_camera_state( const RenderState *state );

private:
	int _aux_bits;
	CPT( RenderState ) _cam_state;
};

#endif // POSTPROCESSSCENEPASS_H
