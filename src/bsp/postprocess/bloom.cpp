/**
 * PANDA3D BSP LIBRARY
 * 
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file bloom.cpp
 * @author Brian Lach
 * @date July 23, 2019
 */

#include "postprocess/bloom.h"
#include "postprocess/postprocess.h"
#include "postprocess/postprocess_pass.h"
#include "postprocess/blur_passes.h"

#include <configVariableDouble.h>
#include <texture.h>

static ConfigVariableDouble r_bloomscale( "r_bloomscale", 1.0 );

static ConfigVariableDouble r_bloomtintr( "r_bloomtintr", 0.3 );
static ConfigVariableDouble r_bloomtintg( "r_bloomtintg", 0.59 );
static ConfigVariableDouble r_bloomtintb( "r_bloomtintb", 0.11 );
static ConfigVariableDouble r_bloomtintexponent( "r_bloomtintexponent", 2.2 );

class DownsampleLuminance : public PostProcessPass
{
public:
	DownsampleLuminance( PostProcess *pp ) :
		PostProcessPass( pp, "bloom-downsample_luminance" ),
		_tap_offsets( PTA_LVecBase2f::empty_array( 4 ) )
	{
		// Downsample by 4
		set_div_size( true, 4 );
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
	}

private:
	PTA_LVecBase2f _tap_offsets;
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

	//
	// Separable gaussian blur
	//

	PT( BlurX ) blur_x = new BlurX( pp, dsl->get_color_texture() );
	blur_x->setup();
	blur_x->add_color_output();

	PT( BlurY ) blur_y = new BlurY( pp, blur_x, LVector3f( r_bloomscale ) );
	blur_y->setup();
	blur_y->add_color_output();

	add_pass( dsl );
	add_pass( blur_x );
	add_pass( blur_y );
}

Texture *BloomEffect::get_final_texture()
{
	return get_pass( "blurY" )->get_color_texture();
}
