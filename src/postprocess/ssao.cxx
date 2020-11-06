/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ssao.cxx
 * @author lachbr
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

static ConfigVariableDouble hbao_sample_radius("hbao-sample-radius", 255.0);
static ConfigVariableDouble hbao_tangent_bias("hbao-tangent-bias", 0.65);
//static ConfigVariableDouble hbao_max_sample_distance("hbao-max-sample-distance", 11.5);
static ConfigVariableDouble hbao_strength("hbao-strength", 1.6);
static ConfigVariableDouble hbao_num_angles("hbao-num-angles", 4);
static ConfigVariableDouble hbao_num_ray_steps("hbao-num-ray-steps", 3);
static ConfigVariableDouble hbao_noise_scale("hbao-noise-scale", 1);
static ConfigVariableInt hbao_noise_size("hbao-noise-size", 64);

static ConfigVariableDouble alchemy_max_distance("alchemy-max-distance", 4.75);
static ConfigVariableDouble alchemy_sample_radius("alchemy-sample-radius", 210.0);

static ConfigVariableDouble ao_blur_normal_factor("ao-blur-normal-factor", 1.2);
static ConfigVariableDouble ao_blur_depth_factor("ao-blur-depth-factor", 0.9);
static ConfigVariableInt ao_clip_length("ao-clip-length", 2);
static ConfigVariableDouble ao_z_bias("ao-z-bias", 0.1);

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
		get_quad().set_shader_input( "depthSampler", _pp->get_scene_depth_texture() );
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

		PNMImage image( res, res, 4 );

		for ( int y = 0; y < res; y++ )
		{
			for ( int x = 0; x < res; x++ )
			{
				LColorf rgba( random.random_real( 1.0 ),
					      random.random_real( 1.0 ),
					      random.random_real( 1.0 ),
					      random.random_real( 1.0 ) );

				image.set_xel_a( x, y, rgba );
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
		quad.set_shader_input("depthSampler", _pp->get_scene_depth_texture());
		quad.set_shader_input("normalSampler", _pp->get_scene_pass()->get_aux_texture(AUXTEXTURE_NORMAL));
		quad.set_shader_input("noiseSampler", _noise_texture);

		update_dynamic_inputs();
		/*
		float r = (float)r_hbao_radius;
		float r2 = r * r;
		float inv_r2 = -1.0f / r2;
		get_quad().set_shader_input("AOStrength_R_R2_NegInvR2",
			LVector4f(r_hbao_strength.get_value(), r, r2, inv_r2));

		get_quad().set_shader_input("NumDirections_Samples",
			LVector2i(r_hbao_dirs.get_value(), r_hbao_samples.get_value()));

		get_quad().set_shader_input("depthSampler", _pp->get_scene_depth_texture());
		get_quad().set_shader_input("noiseSampler", _noise_texture);

		update_dynamic_inputs();
		*/
	}

	void update_dynamic_inputs()
	{
		NodePath quad = get_quad();

		//LVector2i dim = get_back_buffer_dimensions();
		//quad.set_shader_input("WindowSize", dim);

		Lens *lens = get_scene_lens();
		//quad.set_shader_input("NearFar", LVector2(lens->get_near(), lens->get_far()));

		//quad.set_shader_input("SampleRadius_MaxDistance_AOStrength_ClipLength",
		//	LVector4(alchemy_sample_radius.get_value(), alchemy_max_distance.get_value(),
		//					 hbao_strength.get_value(), ao_clip_length.get_value()));

		//quad.set_shader_input("ZBias_ClipLength", LVector2(ao_z_bias.get_value(), ao_clip_length.get_value()));

#if 1 // HBAO

		quad.set_shader_input("FOV_SampleRadius_AngleBias_Intensity",
			LVector4(deg_2_rad(lens->get_fov()[0]), hbao_sample_radius.get_value(),
							 hbao_tangent_bias.get_value(), hbao_strength.get_value()));
		quad.set_shader_input("SampleDirections_SampleSteps_NoiseScale",
			LVector4(hbao_num_angles.get_value(), hbao_num_ray_steps.get_value(),
							 hbao_noise_scale.get_value(), hbao_noise_scale.get_value()));


		//quad.set_shader_input("SampleRadius_TangentBias_MaxSampleDistance_AOStrength",
		//	LVector4(hbao_sample_radius.get_value(), hbao_tangent_bias.get_value(),
		//					 hbao_max_sample_distance.get_value(), hbao_strength.get_value()));

		//quad.set_shader_input("NumAngles_NumRaySteps",
		//	LVector2i(hbao_num_angles.get_value(), hbao_num_ray_steps.get_value()));
#endif

#if 0
		LVector2i dim = get_back_buffer_dimensions();
		get_quad().set_shader_input("AORes_Inv",
			LVector4f(dim[0], dim[1], 1.0f / dim[0], 1.0f / dim[1]));

		float noise_res = (float)r_hbao_noise_res;
		get_quad().set_shader_input("NoiseScale_MaxRadiusPixels",
			LVector3f(dim[0] / noise_res, dim[1] / noise_res,
							  r_hbao_max_radius_pixels.get_value()));

		Lens *lens = get_scene_lens();
		float focal_length = lens->get_focal_length();
		float focal_length_y = focal_length / lens->get_aspect_ratio();
		float inv_focal_length = 1.0f / focal_length;
		float inv_focal_length_y = 1.0f / focal_length_y;
		float near = lens->get_near();
		float far = 300.0f;

		float denom = 2.0f * near * far;
		get_quad().set_shader_input("FocalLen_LinMAD",
			LVector4f(focal_length, focal_length_y,
								(near - far) / denom,
								(near + far) / denom));

		get_quad().set_shader_input("UVToViewA_B",
			LVector4f(-2.0f * inv_focal_length, -2.0f * inv_focal_length_y,
								1.0f * inv_focal_length, 1.0f * inv_focal_length_y));
#endif

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

	const int blur_passes = 5;

	_final_texture = ao_output;

	for (int i = 0; i < blur_passes; i++) {
		std::ostringstream xss;
		xss << "aoWeightedBlurX-" << i;
		PT(WeightedBlur) blur_x = new WeightedBlur(pp, xss.str());
		blur_x->_color = _final_texture;
		blur_x->_normals = _pp->get_scene_pass()->get_aux_texture(AUXTEXTURE_NORMAL);
		blur_x->_depth = _pp->get_scene_depth_texture();
		blur_x->_direction.set(1, 0);
		blur_x->setup();
		blur_x->add_color_output();

		std::ostringstream yss;
		yss << "aoWeightedBlurY-" << i;
		PT(WeightedBlur) blur_y = new WeightedBlur(pp, yss.str());
		blur_y->_color = blur_x->get_color_texture();
		blur_y->_normals = _pp->get_scene_pass()->get_aux_texture(AUXTEXTURE_NORMAL);
		blur_y->_depth = _pp->get_scene_depth_texture();
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
