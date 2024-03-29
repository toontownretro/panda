/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file postProcessScenePass.h
 * @author brian
 * @date 2019-07-24
 */

#ifndef POSTPROCESSSCENEPASS_H
#define POSTPROCESSSCENEPASS_H

#include "postProcessPass.h"

#include "renderState.h"

class PostProcess;

class EXPCL_PANDA_POSTPROCESS PostProcessScenePass : public PostProcessPass
{
	DECLARE_CLASS(PostProcessScenePass, PostProcessPass);

PUBLISHED:
	PostProcessScenePass( PostProcess *pp );
	virtual bool setup_buffer();
	virtual void setup();
	virtual void add_color_output();
	virtual void add_depth_output();
	virtual void add_aux_output( int n );

	LColor get_aux_clear_value(int n) const;
	bool needs_aux_clear(int n) const;
	std::string get_aux_identifier(int n) const;

	void setup_scene_camera( int i, int sort = 0 );

	void set_camera_state( const RenderState *state );

private:
	int _aux_bits;
	CPT( RenderState ) _cam_state;
};

#endif // POSTPROCESSSCENEPASS_H
