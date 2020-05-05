#pragma once

#include "postprocess/postprocess_pass.h"

class BlurX : public PostProcessPass
{
public:
	BlurX( PostProcess *pp, Texture *blur_input ) :
		PostProcessPass( pp, "blurX" ),
		_vs_tap_offsets( PTA_LVecBase2f::empty_array( 3 ) ),
		_ps_tap_offsets( PTA_LVecBase2f::empty_array( 3 ) ),
		_blur_input( blur_input )
	{
		set_div_size( true, 4 );
	}

	virtual void setup()
	{
		PostProcessPass::setup();
		get_quad().set_shader( Shader::load( Shader::SL_GLSL, "shaders/postprocess/blur.vert.glsl",
						     "shaders/postprocess/blur.frag.glsl" ) );
		get_quad().set_shader_input( "texSampler",
					     _blur_input );
		get_quad().set_shader_input( "psTapOffsets", _ps_tap_offsets );
		get_quad().set_shader_input( "vsTapOffsets", _vs_tap_offsets );
		get_quad().set_shader_input( "scaleFactor", LVecBase3f( 1, 1, 1 ) );
	}

	virtual void update()
	{
		PostProcessPass::update();

		// Our buffer's size
		int width = _buffer->get_size()[0];
		float dx = 1.0f / width;

		_vs_tap_offsets[0][1] = 0.0f;
		_vs_tap_offsets[0][0] = 1.3366f * dx;

		_vs_tap_offsets[1][1] = 0.0f;
		_vs_tap_offsets[1][0] = 3.4295f * dx;

		_vs_tap_offsets[2][1] = 0.0f;
		_vs_tap_offsets[2][0] = 5.4264f * dx;

		/////////////////////////////////////////

		_ps_tap_offsets[0][1] = 0.0f;
		_ps_tap_offsets[0][0] = 7.4359f * dx;

		_ps_tap_offsets[1][1] = 0.0f;
		_ps_tap_offsets[1][0] = 9.4436f * dx;

		_ps_tap_offsets[2][1] = 0.0f;
		_ps_tap_offsets[2][0] = 11.4401f * dx;
	}

private:
	Texture *_blur_input;

	PTA_LVecBase2f _vs_tap_offsets;
	PTA_LVecBase2f _ps_tap_offsets;
};

class BlurY : public PostProcessPass
{
public:
	BlurY( PostProcess *pp, BlurX *blur_x, const LVector3f &scale_factor ) :
		PostProcessPass( pp, "blurY" ),
		_vs_tap_offsets( PTA_LVecBase2f::empty_array( 3 ) ),
		_ps_tap_offsets( PTA_LVecBase2f::empty_array( 3 ) ),
		_blur_x( blur_x ),
		_scale_factor( scale_factor )
	{
		set_div_size( true, 4 );
	}

	virtual void setup()
	{
		PostProcessPass::setup();
		get_quad().set_shader( Shader::load( Shader::SL_GLSL, "shaders/postprocess/blur.vert.glsl",
						     "shaders/postprocess/blur.frag.glsl" ) );
		get_quad().set_shader_input( "texSampler",
					     _blur_x->get_color_texture() );
		get_quad().set_shader_input( "psTapOffsets", _ps_tap_offsets );
		get_quad().set_shader_input( "vsTapOffsets", _vs_tap_offsets );
		get_quad().set_shader_input( "scaleFactor", _scale_factor );
	}

	virtual void update()
	{
		PostProcessPass::update();

		// Our buffer's size
		int height = _buffer->get_size()[1];
		float dy = 1.0f / height;

		_vs_tap_offsets[0][0] = 0.0f;
		_vs_tap_offsets[0][1] = 1.3366f * dy;

		_vs_tap_offsets[1][0] = 0.0f;
		_vs_tap_offsets[1][1] = 3.4295f * dy;

		_vs_tap_offsets[2][0] = 0.0f;
		_vs_tap_offsets[2][1] = 5.4264f * dy;

		/////////////////////////////////////////

		_ps_tap_offsets[0][0] = 0.0f;
		_ps_tap_offsets[0][1] = 7.4359f * dy;

		_ps_tap_offsets[1][0] = 0.0f;
		_ps_tap_offsets[1][1] = 9.4436f * dy;

		_ps_tap_offsets[2][0] = 0.0f;
		_ps_tap_offsets[2][1] = 11.4401f * dy;
	}

private:
	BlurX *_blur_x;

	LVector3f _scale_factor;
	PTA_LVecBase2f _vs_tap_offsets;
	PTA_LVecBase2f _ps_tap_offsets;
};
