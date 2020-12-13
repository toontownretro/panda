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
#include "lens.h"
#include "motionBlur.h"

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

// Physically-based camera settings.

static ConfigVariableDouble hdr_min_shutter
("hdr-min-shutter", 1.0 / 4000.0,
 PRC_DESC("The minimum shutter speed of the camera in seconds."));
static ConfigVariableDouble hdr_max_shutter
("hdr-max-shutter", 1.0 / 30.0,
 PRC_DESC("The maximum shutter speed of the camera in seconds."));
static ConfigVariableDouble hdr_shutter_speed
("hdr-shutter-speed", 1.0 / 100.0,
 PRC_DESC("Explicit shutter speed if using the shutter priority method."));

static ConfigVariableDouble hdr_min_aperature
("hdr-min-aperature", 1.8,
 PRC_DESC("The minimum camera aperature size."));
static ConfigVariableDouble hdr_max_aperature
("hdr-max-aperature", 22.0,
 PRC_DESC("The maximum camera aperature size."));
static ConfigVariableDouble hdr_aperature_size
("hdr-aperature-size", 5.0,
 PRC_DESC("Explicit aperature size if using the aperature priority method."));

static ConfigVariableDouble hdr_min_iso
("hdr-min-iso", 100.0,
 PRC_DESC("The minimum camera ISO value."));
static ConfigVariableDouble hdr_max_iso
("hdr-max-iso", 6400.0,
 PRC_DESC("The maximum camera ISO value."));

static ConfigVariableInt hdr_exposure_auto_method
("hdr-exposure-auto-method", 0,
 PRC_DESC("The method used to automatically calculate camera settings from "
          "a luminance value.  0 for program auto, 1 for shutter priority, "
					"2 for aperature priority."));

static ConfigVariableInt hdr_exposure_method
("hdr-exposure-method", 0,
	PRC_DESC("The method used to calculate exposure from the camera's aperature, "
	         "shutter speed, and ISO value.  0 for Saturation-based Speed "
					 "method, 1 for Standard Output Sensitivity method."));

static ConfigVariableDouble hdr_exposure_std_middle_grey
("hdr-exposure-std-middle-grey", 0.18,
 PRC_DESC("The middle grey value to use in the calculation of exposure "
          "using the Standard Output Sensitivity method."));

ConfigVariableInt hdr_num_buckets
("hdr-num-buckets", 256,
 PRC_DESC("The total number of buckets in the HDR luminance histogram."));
ConfigVariableInt hdr_num_ldr_buckets
("hdr-num-ldr-buckets", 16,
 PRC_DESC("The number of buckets in the HDR luminance histogram that are "
          "in low dynamic range (0-1)."));
static ConfigVariableDouble hdr_bucket_range_exponent
("hdr-bucket-range-exponent", 3.0,
 PRC_DESC("The exponential increase of the luminance range of each HDR "
          "histogram bucket."));

static ConfigVariableDouble hdr_luminance_adapation_rate
("hdr-luminance-adapation-rate", 1.0,
 PRC_DESC("Rate at which the average luminance is smoothly adjusted.  Higher "
          "is faster."));

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
	_quad_geom( nullptr ),
	_luminance(0.5f),
	_aperature(4.0f),
	_shutter_speed(1/60.0f),
	_iso(800.0f),
	_max_luminance(1.0f),
	_exposure(1.0f)
{
	// Look at a shrunk-down version of the framebuffer to reduce overhead
	// of the luminance_compare shader, which does a branch and discard.
	set_forced_size( true, ( 128, 128 ) );

	// We aren't actually outputting anything.
	_fbprops.set_rgb_color(false);
	_fbprops.set_float_color(false);
	_fbprops.set_rgba_bits(0, 0, 0, 0);

	_current_bucket = 0;
	_buckets.clear();

	for ( int i = 0; i < hdr_num_buckets; i++ )
	{
		// Use even distribution
		float bmin = i / (float)hdr_num_ldr_buckets;
		float bmax = ( i + 1 ) / (float)hdr_num_ldr_buckets;

		// Use a distribution with slightly more bins in the low range.
		if ( bmin > 0.0f )
		{
			bmin = powf( bmin, (float)hdr_bucket_range_exponent );
		}
		if ( bmax > 0.0f )
		{
			bmax = powf( bmax, (float)hdr_bucket_range_exponent );
		}

		hdrbucket_t bucket;
		bucket.luminance_min = bmin;
		bucket.luminance_max = bmax;
		bucket.ctx = nullptr;
		bucket.pixels = 0;
		_buckets.push_back(bucket);
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
	for ( int i = hdr_num_buckets - 1; i >= 0; i-- )
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

/**
 * Returns the log average value of the luminance histogram.
 */
float HDRPass::
get_average_histogram_luminance(int total_pixels) const {
	float average = 0.0f;
	for (int i = 0; i < hdr_num_buckets; i++) {
		const hdrbucket_t *bucket = &_buckets[i];
		float bin_center = (bucket->luminance_min + bucket->luminance_max) / 2.0f;
		average += bucket->pixels * bin_center;
	}
	average /= total_pixels;
	return average;
}

/**
 * Given an aperature, shutter speed, and exposure value, computes the required
 * ISO value.
 */
float HDRPass::
compute_iso(float aperature, float shutter_speed, float ev) const {
	return (std::pow(aperature, 2.0f) * 100.0f) / (shutter_speed * std::pow(2.0f, ev));
}

/**
 * Given the camera settings, compute the current exposure value.
 */
float HDRPass::
compute_ev(float aperature, float shutter_speed, float iso) const {
	return std::log2((std::pow(aperature, 2.0f) * 100.0f) / (shutter_speed * iso));
}

/**
 * Using the light metering equation, compute the target exposure.
 */
float HDRPass::
compute_target_ev(float average_luminance) const {
	static const float K = 12.5f;
	return std::log2(average_luminance * 100.0f / K);
}

/**
 * Uses the aperature priority method to compute the appropriate camera
 * settings for the given target exposure value.
 */
void HDRPass::
apply_aperature_priority(float focal_length, float target_ev, float &aperature,
												 float &shutter_speed, float &iso) {
	// Start with the assumption that we want a shutter speed of 1/f.
  shutter_speed = 1.0f / focal_length;

	// Compute the resulting ISO if we left the shutter speed here.
	iso = clamp(compute_iso(aperature, shutter_speed, target_ev),
							(float)hdr_min_iso, (float)hdr_max_iso);

	// Figure out how far we were from the target exposure value.
	float ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);

	// Compute the final shutter speed.
	shutter_speed = clamp(shutter_speed * std::pow(2.0f, -ev_diff),
											  (float)hdr_min_shutter, (float)hdr_max_shutter);
}

/**
 * Uses the shutter priority method to compute the appropriate camera
 * settings for the given target exposure value.
 */
void HDRPass::
apply_shutter_priority(float focal_length, float target_ev, float &aperature,
                       float &shutter_speed, float &iso) {
	// Start with the assumption that we want an aperature of 4.0.
	aperature = 4.0f;

	// Compute the resulting ISO if we left the aperature here.
	iso = clamp(compute_iso(aperature, shutter_speed, target_ev),
							(float)hdr_min_iso, (float)hdr_max_iso);

	// Figure out how far we were from the target exposure value.
	float ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);

	// Compute the final aperature.
	aperature = clamp(aperature * std::pow(std::sqrt(2.0f), ev_diff),
										(float)hdr_min_aperature, (float)hdr_max_aperature);
}

/**
 * Uses the program auto method to compute the appropriate camera
 * settings for the given target exposure value.
 */
void HDRPass::
apply_program_auto(float focal_length, float target_ev, float &aperature,
                   float &shutter_speed, float &iso) {
	// Start with the assumption that we want an aperture of 4.0.
	aperature = 4.0f;
	// Start with the assumption that we want a shutter speed of 1/f.
	shutter_speed = 1.0f / focal_length;
	// Compute the resulting ISO if we left both shutter and aperture here.
	iso = clamp(compute_iso(aperature, shutter_speed, target_ev),
							(float)hdr_min_iso, (float)hdr_max_iso);

	// Apply half the difference in EV to the aperture.
	float ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);
	aperature = clamp(aperature * std::pow(std::sqrt(2.0f), ev_diff * 0.5f),
										(float)hdr_min_aperature, (float)hdr_max_aperature);

	// Apply the remaining difference to the shutter speed.
	ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);
	shutter_speed = clamp(shutter_speed * std::pow(2.0f, -ev_diff),
											  (float)hdr_min_shutter, (float)hdr_max_shutter);
}

/**
 * Get an exposure using the Saturation-based Speed method.
 */
float HDRPass::
get_saturation_based_exposure(float aperature, float shutter_speed, float iso) const {
	return 1.0f / _max_luminance;
}

/**
 * Get an exposure using the Standard Output Sensitivity method.
 */
float HDRPass::
get_standard_output_based_exposure(float aperature, float shutter_speed,
															     float iso, float middle_grey) const {
  float l_avg = (1000.0f / 65.0f) * std::pow(aperature, 2.0f) / (iso * shutter_speed);
	return middle_grey / l_avg;
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
	for ( int i = 0; i < hdr_num_buckets; i++ ) {
		total_pixels += _buckets[i].pixels;
	}

	//float percent_location_of_target =
	//	find_location_of_percent_bright_pixels(
	//		hdr_percent_bright_pixels, hdr_percent_target, total_pixels );

	//if ( percent_location_of_target < 0.0f ) {
		// This is the return error code.
		// Pretend we're at the target.
	//	percent_location_of_target = 0.5f;
	//}

	// Make sure this is > 0
	//percent_location_of_target = std::max( 0.0001f, percent_location_of_target );

	// Compute target scalar
	//float target_lum = percent_location_of_target;
	//float target_scalar = percent_location_of_target * ( hdr_percent_target / 100.0f );

	// Compute secondary target scalar
	//float target_lum = find_location_of_percent_bright_pixels(
	//	50.0f, -1.0f, total_pixels );
	float target_lum = get_average_histogram_luminance(total_pixels);
	//if ( avg_lum_location > 0.0f ) {

		// Only override if it's trying to brighten the image more than the primary algorithm
	//	if ( avg_lum_location < target_lum ) {
	//		target_lum = avg_lum_location;
	//	}
	//}

	//if (target_lum < 0.0f) {
	//	target_lum = 0.5f;
	//}

	target_lum = std::max( 0.0001f, target_lum );

	/*
	if (in_average < moving_average_size) {
		moving_average_tone_map_scale[in_average++] = target_lum;
	} else {
		// Scroll, losing oldest
		for (int i = 0; i < moving_average_size - 1; i++) {
			moving_average_tone_map_scale[i] = moving_average_tone_map_scale[i + 1];
		}
		moving_average_tone_map_scale[moving_average_size - 1] = target_lum;
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
		goal_scale = goal_scale;
	}

	float elapsed_time = (float)global_clock->get_dt();
	float rate = hdr_manual_tonemap_rate.get_value() * 2;

	if (rate == 0.0f) {
		// Zero indicates intantaneous tonemap scaling
		_luminance = goal_scale;
	} else {
		if (goal_scale > _luminance) {
			float acc_exposure_adjust = hdr_accelerate_adjust_exposure_down.get_value();
			// Adjust at up to 4x rate when over-exposed.
			rate = std::min(acc_exposure_adjust * rate,
				flerp(rate, acc_exposure_adjust * rate, 0.0f, 1.5f,
					_luminance - goal_scale));
		}

		float rate_x_time = rate * elapsed_time;
		// Limit the rate based on the number of bins to help reduce the exposure
		// scalar "riding the wave" of the histogram re-building.
		rate_x_time = std::min(rate_x_time, (1.0f / (float)hdr_num_buckets) * 0.25f);

		float alpha = std::max(0.0f, std::min(1.0f, rate_x_time));

		_luminance = (goal_scale * alpha) + (_luminance * (1.0f - alpha));
	}
	*/

	// Time average the instantaneous luminance.
	float dt = (float)global_clock->get_dt();
	_luminance = _luminance + (target_lum - _luminance) * (1 - std::exp(-dt * (float)hdr_luminance_adapation_rate));

	NodePath camera_np = _pp->get_camera(0);
	Camera *camera = DCAST(Camera, camera_np.node());
	Lens *lens = camera->get_lens();

	// Now calculate the exposure.
	float target_ev = compute_target_ev(_luminance);
	float aperature, shutter_speed, iso;
	// The focal length of a Panda lens when calculated from FOV is in inches.
	// Convert to millimeters.
	float focal_length = lens->get_focal_length() * 25.4;
	switch (hdr_exposure_auto_method) {
	default:
	case AEM_program_auto:
		apply_program_auto(focal_length, target_ev, aperature, shutter_speed, iso);
		break;
	case AEM_shutter_priority:
		shutter_speed = hdr_shutter_speed.get_value();
		apply_shutter_priority(focal_length, target_ev, aperature, shutter_speed, iso);
		break;
	case AEM_aperature_priority:
		aperature = hdr_aperature_size.get_value();
		apply_aperature_priority(focal_length, target_ev, aperature, shutter_speed, iso);
		break;
	}

	// Compute maximum sensor luminance.
	_max_luminance = (7800.0f / 65.0f) * (std::pow(aperature, 2.0f) / (iso * shutter_speed));

	float exposure;
	switch (hdr_exposure_method) {
	default:
	case EM_saturation_speed:
		exposure = get_saturation_based_exposure(aperature, shutter_speed, iso);
		break;
	case EM_standard_output:
		exposure = get_standard_output_based_exposure(aperature, shutter_speed,
																									iso, hdr_exposure_std_middle_grey);
		break;
	}

	_exposure = exposure;
	_aperature = aperature;
	_shutter_speed = shutter_speed;
	_iso = iso;

	mat_motion_blur_strength = _shutter_speed * 2;

	// Apply the exposure scale to the lens that renders our final screen quad.
	camera_np = _pp->get_scene_pass()->get_camera();
	camera = DCAST(Camera, camera_np.node());
	camera->get_lens()->set_exposure_scale(exposure);
}

void HDRPass::setup_quad()
{
	CardMaker cm(get_name() + "-quad");
	cm.set_frame_fullscreen_quad();
	PT(GeomNode) gn = DCAST(GeomNode, cm.generate());
	_quad_geom = gn->get_geom(0);

	PT(Shader) shader = Shader::load(Shader::SL_GLSL,
							"shaders/postprocess/luminance_compare.vert.glsl",
							"shaders/postprocess/luminance_compare.frag.glsl");
	CPT(RenderAttrib) shattr = ShaderAttrib::make(shader);
	shattr = DCAST(ShaderAttrib, shattr)->set_shader_input("sceneColorSampler", _pp->get_scene_color_texture());

	CPT(RenderState) state = RenderState::make(
		ColorWriteAttrib::make(ColorWriteAttrib::C_all),
		DepthWriteAttrib::make(DepthWriteAttrib::M_off),
		DepthTestAttrib::make(DepthTestAttrib::M_none)
	);

	for (int i = 0; i < hdr_num_buckets; i++) {
		hdrbucket_t &bucket = _buckets[i];

		// Create a Geom and state for each bucket.

		shattr = DCAST(ShaderAttrib, shattr)->set_shader_input("luminanceMinMax",
			LVector2(bucket.luminance_min, bucket.luminance_max));

		state = state->set_attrib(shattr);
		bucket.state = state;
	}

	PT(CallbackNode) node = new CallbackNode("hdr-callback");
	node->set_draw_callback(new HDRCallbackObject(this));
	node->set_bounds(new OmniBoundingVolume);
	_quad_np = NodePath(node);
}

void HDRPass::draw( CallbackData *data )
{

	GeomDrawCallbackData *geom_cbdata;
	DCAST_INTO_V( geom_cbdata, data );
	GraphicsStateGuardian *gsg;
	DCAST_INTO_V( gsg, geom_cbdata->get_gsg() );

	geom_cbdata->set_lost_state( false );

	CullableObject *obj = geom_cbdata->get_object();

	// The original system had 16 buckets and drew 1 bucket per frame.
	// Draw as many buckets as we need to in order to keep the same histogram
	// update rate.
	int draw_buckets = hdr_num_buckets / 16;

	for (int i = 0; i < draw_buckets; i++) {
		_current_bucket++;
		if ( _current_bucket >= hdr_num_buckets ) {
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
					continue;
				}
		}

		gsg->set_state_and_transform( bucket->state, obj->_internal_transform );

		CPT( Geom ) munged_geom = _quad_geom;
		CPT( GeomVertexData ) munged_data = _quad_geom->get_vertex_data();
		gsg->get_geom_munger( bucket->state, Thread::get_current_thread() )->
			munge_geom( munged_geom, munged_data, false, Thread::get_current_thread() );

		// Render the quad.
		// The shader will discard any pixels that are not within the specified luminance range.
		// The number of pixels returned by the occlusion query is the number of pixels in that bucket/luminance range.
		gsg->begin_occlusion_query();
		munged_geom->draw( gsg, munged_data, 1, true, Thread::get_current_thread() );
		bucket->ctx = gsg->end_occlusion_query();
	}
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
