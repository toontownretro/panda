/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ssao.cxx
 * @author brian
 */

#include "ssao.h"
#include "postProcessPass.h"
#include "postProcess.h"
#include "postProcessDefines.h"
#include "blurPasses.h"
#include "deg_2_rad.h"

#include "texturePool.h"
#include "pnmImage.h"
#include "randomizer.h"
#include "configVariableDouble.h"

static ConfigVariableDouble hbao_falloff("hbao-falloff", 0.5);
static ConfigVariableDouble hbao_max_sample_distance("hbao-max-sample-distance", 0.5);
static ConfigVariableDouble hbao_sample_radius("hbao-sample-radius", 0.5);
static ConfigVariableDouble hbao_angle_bias("hbao-angle-bias", 0.65);
static ConfigVariableDouble hbao_strength("hbao-strength", 1.6);
static ConfigVariableDouble hbao_num_angles("hbao-num-angles", 4);
static ConfigVariableDouble hbao_num_ray_steps("hbao-num-ray-steps", 3);
static ConfigVariableInt hbao_noise_size("hbao-noise-size", 4);

static ConfigVariableDouble ao_blur_normal_factor("ao-blur-normal-factor", 1.2);
static ConfigVariableDouble ao_blur_depth_factor("ao-blur-depth-factor", 0.9);
//static ConfigVariableDouble ao_z_bias("ao-z-bias", 0.1);

//static ConfigVariableInt ao_quality("ao-quality", 3);

IMPLEMENT_CLASS( SSAO_Effect )

class WeightedBlur : public PostProcessPass {
public:
	WeightedBlur(PostProcess *pp, const std::string &name = "weightedBlur") :
		PostProcessPass(pp, name),
		_color(nullptr),
		_depth(nullptr),
		_normals(nullptr),
		_direction(LVector2i(1, 0)),
		_pixel_stretch(1.0f)
	{
		// This pass blurs the ambient occlusion texture, which only needs a single
		// 8-bit channel.
		_fbprops.set_float_color(false);
		_fbprops.set_rgba_bits(8, 0, 0, 0);
	}

	virtual void setup() {
		PostProcessPass::setup();

		NodePath quad = get_quad();
		quad.set_shader(
			Shader::load(
				Shader::SL_GLSL,
				"shaders/postprocess/weighted_blur.vert.glsl",
				"shaders/postprocess/weighted_blur.frag.glsl"
			)
		);

		quad.set_shader_input("depthSampler", _depth);
		quad.set_shader_input("normalSampler", _normals);
		quad.set_shader_input("colorSampler", _color);
		quad.set_shader_input("blurDirection", _direction);
		quad.set_shader_input("pixelStretch_normalFactor_depthFactor",
			LVector3(_pixel_stretch, ao_blur_normal_factor.get_value(),
							 ao_blur_depth_factor.get_value()));

		update_dynamic_inputs();
	}

	void update_dynamic_inputs() {
		get_quad().set_shader_input("screenSize", get_back_buffer_dimensions());
	}

	virtual void update() {
		PostProcessPass::update();
		update_dynamic_inputs();
	}

public:
	PT(Texture) _color;
	PT(Texture) _depth;
	PT(Texture) _normals;
	LVector2i _direction;
	float _pixel_stretch;
};

class SSAO_Pass : public PostProcessPass
{
public:
	SSAO_Pass( PostProcess *pp ) :
		PostProcessPass( pp, "ao-pass" ),
		_dimensions( PTA_LVecBase2f::empty_array( 1 ) )
	{
		// We only need a single 8-bit channel for ambient occlusion.
		_fbprops.set_float_color(false);
		_fbprops.set_rgba_bits(8, 0, 0, 0);
	}

	virtual void setup()
	{
		PostProcessPass::setup();

		get_quad().set_shader(
			Shader::load(
				Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/ssao.frag.glsl"
			)
		);
		get_quad().set_shader_input( "depthSampler", _pp->get_output_pipe("scene_depth") );
		get_quad().set_shader_input( "resolution", _dimensions );
		get_quad().set_shader_input( "near_far_minDepth_radius", LVector4f( 1, 100, 0.3f, 5.0f ) );
		get_quad().set_shader_input( "noiseAmount_diffArea_gDisplace_gArea", LVector4f( 0.0003f, 0.4f, 0.4f, 2.0f ) );
	}

	virtual void update()
	{
		PostProcessPass::update();

		LVector2i dim = get_back_buffer_dimensions();
		_dimensions[0][0] = dim[0];
		_dimensions[0][1] = dim[1];
	}

	PTA_LVecBase2f _dimensions;
};

class HBAO_Pass : public PostProcessPass
{
public:
	HBAO_Pass( PostProcess *pp ) :
		PostProcessPass( pp, "ao-pass" ),
		_noise_texture( nullptr )
	{
		// We only need a single 8-bit channel for ambient occlusion.
		_fbprops.set_float_color(false);
		_fbprops.set_rgba_bits(8, 0, 0, 0);
	}

	void generate_noise_texture( int res )
	{
		Randomizer random;

		PNMImage image( res, res, 3 );

		for ( int y = 0; y < res; y++ )
		{
			for ( int x = 0; x < res; x++ )
			{
				float val = random.random_real(1.0);
				LRGBColorf rgb(std::cos(val), std::sin(val),
					      			 random.random_real(1.0));

				image.set_xel(x, y, rgb);
			}
		}

		_noise_texture = new Texture;
		_noise_texture->load( image );
	}

	virtual void setup()
	{
		PostProcessPass::setup();

		generate_noise_texture( hbao_noise_size );

		NodePath quad = get_quad();

		quad.set_shader(
			Shader::load(
				Shader::SL_GLSL,
				"shaders/postprocess/hbao.vert.glsl",
				"shaders/postprocess/hbao2.frag.glsl"
			)
		);

		quad.set_shader_input("camera", _pp->get_camera(0));
		quad.set_shader_input("depthSampler", _pp->get_output_pipe("scene_depth"));
		quad.set_shader_input("normalSampler", _pp->get_output_pipe("scene_normals"));
		quad.set_shader_input("noiseSampler", _noise_texture);

		update_dynamic_inputs();
	}

	void update_dynamic_inputs()
	{
		NodePath quad = get_quad();

		LVector2i dim = get_back_buffer_dimensions();

		float noise_size = hbao_noise_size.get_value();

		quad.set_shader_input("FallOff_SampleRadius_AngleBias_Intensity",
			LVector4(hbao_falloff.get_value(), hbao_sample_radius.get_value(),
							 hbao_angle_bias.get_value(), hbao_strength.get_value()));
		quad.set_shader_input("SampleDirections_SampleSteps_NoiseScale",
			LVector4(hbao_num_angles.get_value(), hbao_num_ray_steps.get_value(),
							 dim[0] / noise_size, dim[1] / noise_size));
		quad.set_shader_input("MaxSampleDistance", LVector2(hbao_max_sample_distance.get_value()));
	}

	virtual void update()
	{
		PostProcessPass::update();

		update_dynamic_inputs();
	}

	PT( Texture ) _noise_texture;
};

SSAO_Effect::SSAO_Effect( PostProcess *pp, Mode mode ) :
	PostProcessEffect( pp )
{
	// We require the depth buffer and normals
	pp->get_scene_pass()->add_depth_output();
	pp->get_scene_pass()->add_aux_output(AUXTEXTURE_NORMAL);

	Texture *ao_output;

	if ( mode == M_SSAO )
	{
		PT( SSAO_Pass ) ssao = new SSAO_Pass( pp );
		ssao->setup();
		ssao->add_color_output();
		ao_output = ssao->get_color_texture();

		add_pass( ssao );
	}
	else // M_HBAO
	{

		PT(HBAO_Pass) hbao = new HBAO_Pass(pp);
		hbao->setup();
		hbao->add_color_output();
		ao_output = hbao->get_color_texture();

		add_pass(hbao);
	}

	//
	// Separable gaussian blur
	//

	const int blur_passes = 3;

	_final_texture = ao_output;

	for (int i = 0; i < blur_passes; i++) {
		std::ostringstream xss;
		xss << "aoWeightedBlurX-" << i;
		PT(WeightedBlur) blur_x = new WeightedBlur(pp, xss.str());
		blur_x->_color = _final_texture;
		blur_x->_normals = _pp->get_output_pipe("scene_normals");
		blur_x->_depth = _pp->get_output_pipe("scene_depth");
		blur_x->_direction.set(1, 0);
		blur_x->setup();
		blur_x->add_color_output();

		std::ostringstream yss;
		yss << "aoWeightedBlurY-" << i;
		PT(WeightedBlur) blur_y = new WeightedBlur(pp, yss.str());
		blur_y->_color = blur_x->get_color_texture();
		blur_y->_normals = _pp->get_output_pipe("scene_normals");
		blur_y->_depth = _pp->get_output_pipe("scene_depth");
		blur_y->_direction.set(0, 1);
		blur_y->setup();
		blur_y->add_color_output();

		_final_texture = blur_y->get_color_texture();

		add_pass(blur_x);
		add_pass(blur_y);
	}

}

Texture *SSAO_Effect::get_final_texture()
{
	return _final_texture;
}
