/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hdr.cxx
 * @author lachbr
 * @date 2019-07-22
 */

#include "hdr.h"
#include "postProcess.h"

#include "mathutil_misc.h"
#include "clockObject.h"
#include "cardMaker.h"
#include "omniBoundingVolume.h"
#include "callbackNode.h"
#include "callbackObject.h"
#include "graphicsBuffer.h"
#include "colorWriteAttrib.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "configVariableDouble.h"
#include "graphicsStateGuardian.h"
#include "geomDrawCallbackData.h"

static ConfigVariableDouble hdr_percent_bright_pixels
("hdr-percent-bright-pixels", 2.0);
static ConfigVariableDouble hdr_min_avg_lum
("hdr-min-avg-lum", 3.0);
static ConfigVariableDouble hdr_percent_target
("hdr-percent-target", 60.0);
static ConfigVariableDouble hdr_tonemap_scale
("hdr-tonemap-scale", 1.0);
static ConfigVariableDouble hdr_exposure_min
("hdr-exposure-min", 0.5);
static ConfigVariableDouble hdr_exposure_max
("hdr-exposure-max", 2.0);
static ConfigVariableDouble hdr_manual_tonemap_rate
("hdr-manual-tonemap-rate", 1.0);
static ConfigVariableDouble hdr_accelerate_adjust_exposure_down
("hdr-accelerate-adjust-exposure-down", 3.0);
ConfigVariableBool hdr_auto_exposure
("hdr-auto-exposure", true);

static constexpr int moving_average_size = 10;
static float moving_average_tone_map_scale[moving_average_size] = {
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};
static int in_average = 0;

class HDRCallbackObject : public CallbackObject
{
public:
	ALLOC_DELETED_CHAIN( HDRCallbackObject );

	HDRCallbackObject( HDRPass *pass ) :
		_pass( pass )
	{
	}

	virtual void do_callback( CallbackData *data )
	{
		_pass->draw( data );
	}

private:
	HDRPass *_pass;
};

IMPLEMENT_CLASS( HDRPass );

HDRPass::HDRPass( PostProcess *pp ) :
	PostProcessPass( pp, "hdr" ),
	_hdr_quad_geom( nullptr ),
	_hdr_geom_state( nullptr ),
	_current_bucket( 0 ),
	_luminance_min_max( PTA_stdfloat::empty_array( 2 ) ),
	_exposure(1.0f)
{
	// Look at a shrunk-down version of the framebuffer to reduce overhead
	// of the luminance_compare shader, which does a branch and discard.
	set_forced_size( true, ( 128, 128 ) );

	// We aren't actually outputting anything.
	_fbprops.set_rgb_color(false);
	_fbprops.set_float_color(false);
	_fbprops.set_rgba_bits(0, 0, 0, 0);

	_luminance_min_max[0] = 0.0f;
	_luminance_min_max[1] = 0.0f;

	for ( int i = 0; i < HDR_NUM_BUCKETS; i++ )
	{
		// Use even distribution
		float bmin = i / (float)HDR_NUM_BUCKETS;
		float bmax = ( i + 1 ) / (float)HDR_NUM_BUCKETS;

		// Use a distribution with slightly more bins in the low range.
		if ( bmin > 0.0f )
		{
			bmin = powf( bmin, 1.5f );
		}
		if ( bmax > 0.0f )
		{
			bmax = powf( bmax, 1.5f );
		}

		hdrbucket_t bucket;
		bucket.luminance_min = bmin;
		bucket.luminance_max = bmax;
		bucket.ctx = nullptr;
		bucket.pixels = 0;
		_buckets[i] = bucket;
	}
}

float HDRPass::find_location_of_percent_bright_pixels(
	float percent_bright_pixels, float same_bin_snap,
	int total_pixel_count )
{
	// Find where percent range border is
	float range_tested = 0.0f;
	float pixels_tested = 0.0f;

	// Starts at bright end
	for ( int i = HDR_NUM_BUCKETS - 1; i >= 0; i-- )
	{
		hdrbucket_t *bucket = &_buckets[i];

		float pixel_percent_needed = ( percent_bright_pixels / 100.0f ) - pixels_tested;
		float bin_percent = bucket->pixels / (float)total_pixel_count;
		float bin_range = bucket->luminance_max - bucket->luminance_min;
		if ( bin_percent >= pixel_percent_needed )
		{
			// We found the bin needed
			if ( same_bin_snap >= 0.0f )
			{
				if ( bucket->luminance_min <= ( same_bin_snap / 100.0f ) &&
				     bucket->luminance_max >= ( same_bin_snap / 100.0f ) )
				{
					// Sticky bin...
					// We're in the same bin as the target, so keep the tonemap scale
					// where it is.
					return ( same_bin_snap / 100.0f );
				}
			}

			float percent_these_needed = pixel_percent_needed / bin_percent;
			float percent_location = 1.0f - ( range_tested + ( bin_range * percent_these_needed ) );
			// Clamp to this bin just in case.
			percent_location = clamp( percent_location, bucket->luminance_min, bucket->luminance_max );
			return percent_location;
		}

		pixels_tested += bin_percent;
		range_tested += bin_range;
	}

	return -1.0f;
}

void HDRPass::setup()
{
	PostProcessPass::setup();
}

void HDRPass::setup_region()
{
	PostProcessPass::setup_region();
	_buffer->set_clear_color_active( false );
}

void HDRPass::update()
{
	PostProcessPass::update();

	if ( !hdr_auto_exposure ) {
		return;
	}

	ClockObject *global_clock = ClockObject::get_global_clock();

	int total_pixels = 0;
	for ( int i = 0; i < HDR_NUM_BUCKETS; i++ ) {
		total_pixels += _buckets[i].pixels;
	}

	float percent_location_of_target =
		find_location_of_percent_bright_pixels(
			hdr_percent_bright_pixels, hdr_percent_target, total_pixels );

	if ( percent_location_of_target < 0.0f ) {
		// This is the return error code.
		// Pretend we're at the target.
		percent_location_of_target = hdr_percent_target / 100.0f;
	}

	// Make sure this is > 0
	percent_location_of_target = std::max( 0.0001f, percent_location_of_target );

	// Compute target scalar
	float target_scalar = ( hdr_percent_target / 100.0f ) / percent_location_of_target;

	// Compute secondary target scalar
	float avg_lum_location = find_location_of_percent_bright_pixels(
		50.0f, -1.0f, total_pixels );
	if ( avg_lum_location > 0.0f ) {
		float target_scalar_2 = ( hdr_min_avg_lum / 100.0f ) / avg_lum_location;

		// Only override if it's trying to brighten the image more than the primary algorithm
		if ( target_scalar_2 > target_scalar ) {
			target_scalar = target_scalar_2;
		}
	}

	target_scalar = std::max( 0.001f, target_scalar );

	if (in_average < moving_average_size) {
		moving_average_tone_map_scale[in_average++] = target_scalar;
	} else {
		// Scroll, losing oldest
		for (int i = 0; i < moving_average_size - 1; i++) {
			moving_average_tone_map_scale[i] = moving_average_tone_map_scale[i + 1];
		}
		moving_average_tone_map_scale[moving_average_size - 1] = target_scalar;
	}

	// Now, use the average of the last tonemap calculations as our goal scale.
	float goal_scale = 0.0f;
	if (in_average == moving_average_size) {
		float sum_weights = 0.0f;
		int sample_point = moving_average_size / 2;

		for (int i = 0; i < moving_average_size; i++) {
			float weight = std::abs(i - sample_point) / sample_point;
			sum_weights += weight;
			goal_scale += weight * moving_average_tone_map_scale[i];
		}

		goal_scale /= sum_weights;
		goal_scale = std::min((float)hdr_exposure_max,
			std::max((float)hdr_exposure_min, goal_scale));
	}

	float elapsed_time = (float)global_clock->get_dt();
	float rate = hdr_manual_tonemap_rate.get_value() * 2;

	if (rate == 0.0f) {
		// Zero indicates intantaneous tonemap scaling
		_exposure = goal_scale;
	} else {
		if (goal_scale < _exposure) {
			float acc_exposure_adjust = hdr_accelerate_adjust_exposure_down.get_value();
			// Adjust at up to 4x rate when over-exposed.
			rate = std::min(acc_exposure_adjust * rate,
				flerp(rate, acc_exposure_adjust * rate, 0.0f, 1.5f,
					_exposure - goal_scale));
		}

		float rate_x_time = rate * elapsed_time;
		// Limit the rate based on the number of bins to help reduce the exposure
		// scalar "riding the wave" of the histogram re-building.
		rate_x_time = std::min(rate_x_time, (1.0f / HDR_NUM_BUCKETS) * 0.25f);

		float alpha = std::max(0.0f, std::min(1.0f, rate_x_time));

		_exposure = (goal_scale * alpha) + (_exposure * (1.0f - alpha));
	}

	// Apply the exposure scale to the lens that renders our final screen quad.
	NodePath camera_np = _pp->get_scene_pass()->get_camera();
	Camera *camera = DCAST(Camera, camera_np.node());
	camera->get_lens()->set_exposure_scale(_exposure);
}

void HDRPass::setup_quad()
{
	CardMaker cm( get_name() + "-quad" );
	cm.set_frame_fullscreen_quad();
	PT( GeomNode ) gn = DCAST( GeomNode, cm.generate() );
	_hdr_quad_geom = gn->get_geom( 0 );

	PT( Shader ) shader = Shader::load( Shader::SL_GLSL,
					    "shaders/postprocess/luminance_compare.vert.glsl",
					    "shaders/postprocess/luminance_compare.frag.glsl" );
	CPT( RenderAttrib ) shattr = ShaderAttrib::make( shader );
	shattr = DCAST( ShaderAttrib, shattr )->set_shader_input( "sceneColorSampler", _pp->get_scene_color_texture() );
	shattr = DCAST( ShaderAttrib, shattr )->set_shader_input( "luminanceMinMax", _luminance_min_max );

	_hdr_geom_state = RenderState::make(
		shattr, ColorWriteAttrib::make( ColorWriteAttrib::C_all ),
		DepthWriteAttrib::make( DepthWriteAttrib::M_off ),
		DepthTestAttrib::make( DepthTestAttrib::M_none )
	);

	PT( CallbackNode ) node = new CallbackNode( "hdr-callback" );
	node->set_draw_callback( new HDRCallbackObject( this ) );
	node->set_bounds( new OmniBoundingVolume );
	_quad_np = NodePath( node );
}

void HDRPass::draw( CallbackData *data )
{

	GeomDrawCallbackData *geom_cbdata;
	DCAST_INTO_V( geom_cbdata, data );
	GraphicsStateGuardian *gsg;
	DCAST_INTO_V( gsg, geom_cbdata->get_gsg() );

	geom_cbdata->set_lost_state( false );

	CullableObject *obj = geom_cbdata->get_object();

	_current_bucket++;
	if ( _current_bucket >= HDR_NUM_BUCKETS ) {
		_current_bucket = 0;
	}
	hdrbucket_t *bucket = &_buckets[_current_bucket];

	if ( bucket->ctx )
	{
			if ( bucket->ctx->is_answer_ready() )
			{
				bucket->pixels = bucket->ctx->get_num_fragments();
				bucket->ctx = nullptr;
			}
			else
			{
				return;
			}
	}

	_buffer->set_clear_color_active( true );
	_buffer->clear( Thread::get_current_thread() );
	_buffer->set_clear_color_active( false );

	// Adjust the bucket ranges for the shader.
	_luminance_min_max[0] = bucket->luminance_min;
	if (_current_bucket == (HDR_NUM_BUCKETS - 1)) {
		// If we're in the highest luminance bucket, give the shader a higher max
		// luminance so we can catch pixels that are brighter than 1.
		_luminance_min_max[1] = 10000.0f;
	} else {
		_luminance_min_max[1] = bucket->luminance_max;
	}

	gsg->set_state_and_transform( _hdr_geom_state, obj->_internal_transform );

	CPT( Geom ) munged_geom = _hdr_quad_geom;
	CPT( GeomVertexData ) munged_data = _hdr_quad_geom->get_vertex_data();
	gsg->get_geom_munger( _hdr_geom_state, Thread::get_current_thread() )->
		munge_geom( munged_geom, munged_data, false, Thread::get_current_thread() );

	// Render the quad.
	// The shader will discard any pixels that are not within the specified luminance range.
	// The number of pixels returned by the occlusion query is the number of pixels in that bucket/luminance range.
	gsg->begin_occlusion_query();
	munged_geom->draw( gsg, munged_data, true, Thread::get_current_thread() );
	bucket->ctx = gsg->end_occlusion_query();
}

HDREffect::HDREffect( PostProcess *pp ) :
	PostProcessEffect( pp, "hdr" )
{
	PT( HDRPass ) pass = new HDRPass( pp );
	pass->setup();
	pass->add_color_output();

	add_pass( pass );
}

Texture *HDREffect::get_final_texture()
{
	return get_pass( "hdr" )->get_color_texture();
}
