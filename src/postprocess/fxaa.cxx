/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fxaa.cxx
 * @author lachbr
 * @date 2019-12-07
 */

#include "fxaa.h"
#include "postProcessPass.h"
#include "postProcess.h"

class FXAA_Pass : public PostProcessPass
{
	DECLARE_CLASS(FXAA_Pass, PostProcessPass);

public:
	FXAA_Pass( PostProcess *pp ) :
		PostProcessPass( pp, "fxaa-pass" )
	{
		_fbprops.set_rgba_bits(16, 16, 16, 16);
	}

	virtual void setup()
	{
		PostProcessPass::setup();
		get_quad().set_shader(
			Shader::load(
				Shader::SL_GLSL,
				"shaders/postprocess/fxaa.vert.glsl",
				"shaders/postprocess/fxaa.frag.glsl"
			)
		);
		get_quad().set_shader_input( "sceneTexture", _pp->get_output_pipe("scene_color") );
	}
};

IMPLEMENT_CLASS(FXAA_Pass);
IMPLEMENT_CLASS( FXAA_Effect );

FXAA_Effect::FXAA_Effect( PostProcess *pp ) :
	PostProcessEffect( pp, "fxaa" )
{
	FXAA_Pass::init_type();

	PT( FXAA_Pass ) pass = new FXAA_Pass( pp );
	pass->setup();
	pass->add_color_output();
	pp->push_output_pipe("scene_color", pass->get_color_texture());

	add_pass( pass );
}

Texture *FXAA_Effect::get_final_texture()
{
	return get_pass( "fxaa-pass" )->get_color_texture();
}
