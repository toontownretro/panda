#include "postprocess/ssao.h"
#include "postprocess/postprocess_pass.h"
#include "postprocess/postprocess.h"
#include "postprocess/blur_passes.h"

#include <texturePool.h>
#include <pnmImage.h>
#include <randomizer.h>
#include <configVariableDouble.h>

static ConfigVariableDouble r_hbao_radius( "r_hbao_radius", 0.3 );
static ConfigVariableDouble r_hbao_strength( "r_hbao_strength", 2.5 );
static ConfigVariableDouble r_hbao_max_radius_pixels( "r_hbao_max_radius_pixels", 50.0 );
//static ConfigVariableInt r_hbao_res_ratio( "r_hbao_res_ratio", 2 );
static ConfigVariableInt r_hbao_dirs( "r_hbao_dirs", 6 );
static ConfigVariableInt r_hbao_samples( "r_hbao_samples", 3 );
static ConfigVariableInt r_hbao_noise_res( "r_hbao_noise_res", 4 );

IMPLEMENT_CLASS( SSAO_Effect )

class SSAO_Pass : public PostProcessPass
{
public:
	SSAO_Pass( PostProcess *pp ) :
		PostProcessPass( pp, "ssao-pass" ),
		_dimensions( PTA_LVecBase2f::empty_array( 1 ) )
	{
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
		PostProcessPass( pp ),
		_noise_texture( nullptr )
	{

	}

	void generate_noise_texture( int res )
	{
		Randomizer random;

		PNMImage image( res, res, 4 );

		for ( int y = 0; y < res; y++ )
		{
			for ( int x = 0; x < res; x++ )
			{
				LColorf rgba( std::cos( random.random_real( 1.0 ) ),
					      std::sin( random.random_real( 1.0 ) ),
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

		generate_noise_texture( r_hbao_noise_res );

		get_quad().set_shader(
			Shader::load(
				Shader::SL_GLSL,
				"shaders/postprocess/base.vert.glsl",
				"shaders/postprocess/hbao.frag.glsl"
				)
			);
		update_dynamic_inputs();
	}

	void update_dynamic_inputs()
	{
		LVector2i dim = get_back_buffer_dimensions();
		get_quad().set_shader_input( "AORes_Inv", LVector4f( dim[0], dim[1], 1.0f / dim[0], 1.0f / dim[1] ) );
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
	// We require the depth buffer
	pp->get_scene_pass()->add_depth_output();

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

	}
	

	//
	// Separable gaussian blur
	//

	PT( BlurX ) blur_x = new BlurX( pp, ao_output );
	blur_x->setup();
	blur_x->add_color_output();

	PT( BlurY ) blur_y = new BlurY( pp, blur_x, 1 );
	blur_y->setup();
	blur_y->add_color_output();

	
	add_pass( blur_x );
	add_pass( blur_y );
}

Texture *SSAO_Effect::get_final_texture()
{
	return get_pass( "ssao-pass" )->get_color_texture();
}
