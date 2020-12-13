/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bloom.cxx
 * @author lachbr
 * @date 2019-07-23
 */

#include "bloom.h"
#include "postProcess.h"
#include "postProcessPass.h"
#include "blurPasses.h"
#include "hdr.h"

#include "configVariableDouble.h"
#include "texture.h"

static ConfigVariableDouble r_bloomscale( "r_bloomscale", 1.0 );

static ConfigVariableDouble r_bloomtintr( "r_bloomtintr", 0.3 );
static ConfigVariableDouble r_bloomtintg( "r_bloomtintg", 0.59 );
static ConfigVariableDouble r_bloomtintb( "r_bloomtintb", 0.11 );
static ConfigVariableDouble r_bloomtintexponent( "r_bloomtintexponent", 2.2 );
static ConfigVariableInt bloom_blur_passes("bloom-blur-passes", 3);
static ConfigVariableInt bloom_downsample_factor("bloom-downsample-factor", 4);

class DownsampleLuminance : public PostProcessPass
{
public:
	DownsampleLuminance( PostProcess *pp ) :
		PostProcessPass( pp, "bloom-downsample_luminance" ),
		_tap_offsets( PTA_LVecBase2f::empty_array( 4 ) ),
		_cam_settings(PTA_LVecBase2f::empty_array(1))
	{
		// Downsample by 4
		set_div_size( true, bloom_downsample_factor );
		_fbprops.set_alpha_bits(0);

		_hdr = ((HDREffect *)_pp->get_effect("hdr"))->get_hdr_pass();
	}

	virtual void setup()
	{
		PostProcessPass::setup();
		get_quad().set_shader( Shader::load( Shader::SL_GLSL, "shaders/postprocess/downsample.vert.glsl",
						     "shaders/postprocess/downsample.frag.glsl" ) );

		// Vertex shader
		get_quad().set_shader_input( "tapOffsets", _tap_offsets );

		// Pixel shader
		get_quad().set_shader_input( "fbColorSampler", _pp->get_scene_pass()->get_aux_texture( AUXTEXTURE_BLOOM ) );
		get_quad().set_shader_input( "params", LVecBase4f( r_bloomtintr,
								   r_bloomtintg,
								   r_bloomtintb,
								   r_bloomtintexponent ) );

		// Apply the camera settings calculated in the HDR pass.  The exposure
		// scale and max luminance are important to the bloom calculation.
		get_quad().set_shader_input("camSettings", _cam_settings);
	}

	virtual void update()
	{
		PostProcessPass::update();

		// Size of the backbuffer/GraphicsWindow
		LVector2i bb_size = get_back_buffer_dimensions();
		float dx = 1.0f / bb_size[0];
		float dy = 1.0f / bb_size[1];

		_tap_offsets[0][0] = 0.5f * dx;
		_tap_offsets[0][1] = 0.5f * dy;

		_tap_offsets[1][0] = 2.5f * dx;
		_tap_offsets[1][1] = 0.5f * dy;

		_tap_offsets[2][0] = 0.5f * dx;
		_tap_offsets[2][1] = 2.5f * dy;

		_tap_offsets[3][0] = 2.5f * dx;
		_tap_offsets[3][1] = 2.5f * dy;

		_cam_settings[0][0] = _hdr->get_exposure();
		_cam_settings[0][1] = _hdr->get_max_luminance();
	}

private:
	PTA_LVecBase2f _tap_offsets;
	PTA_LVecBase2f _cam_settings;
	HDRPass *_hdr;
};

IMPLEMENT_CLASS( BloomEffect );

BloomEffect::BloomEffect( PostProcess *pp ) :
	PostProcessEffect( pp, "bloom" )
{
	// Ensure we have the scene pass output we need
	pp->get_scene_pass()->add_aux_output( AUXTEXTURE_BLOOM );

	// Downsample the framebuffer by 4, multiply image by luminance of image
	PT( DownsampleLuminance ) dsl = new DownsampleLuminance( pp );
	dsl->setup();
	dsl->add_color_output();

	add_pass( dsl );

	//
	// Separable gaussian blur
	//

	_final_texture = dsl->get_color_texture();

	for (int i = 0; i < bloom_blur_passes; i++) {
		PT( BlurX ) blur_x = new BlurX( pp, _final_texture );
		blur_x->setup();
		blur_x->add_color_output();

		PT( BlurY ) blur_y = new BlurY( pp, blur_x, LVector3f( r_bloomscale ) );
		blur_y->setup();
		blur_y->add_color_output();

		_final_texture = blur_y->get_color_texture();

		add_pass( blur_x );
		add_pass( blur_y );
	}
}

Texture *BloomEffect::get_final_texture()
{
	return _final_texture;
}
