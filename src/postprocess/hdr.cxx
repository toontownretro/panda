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
//#include "motionBlur.h"
#include "graphicsEngine.h"

static constexpr int hdr_num_buckets = 256;
static constexpr int hdr_work_group_size = 16;

ConfigVariableBool hdr_auto_exposure
("hdr-auto-exposure", true);

ConfigVariableInt hdr_luminance_buffers
("hdr-luminance-buffers", 2,
 PRC_DESC("The number of buffers in the luminance compute swap-chain.  Setting "
          "this value higher may reduce pipeline stall from reading in the "
					"results of the luminance compute shader, but the results will be "
					"more delayed."));

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

static ConfigVariableInt hdr_iso_value
("hdr-iso-value", 3200,
 PRC_DESC("ISO value for the exposure calculation."));

static ConfigVariableDouble hdr_min_ev
("hdr-min-ev", 1.0,
 PRC_DESC("The minimum exposure value."));

static ConfigVariableDouble hdr_max_ev
("hdr-max-ev", 20.0,
 PRC_DESC("The maximum exposure value."));

static ConfigVariableInt hdr_exposure_method
("hdr-exposure-method", 0,
	PRC_DESC("The method used to calculate exposure from the camera's aperature, "
	         "shutter speed, and ISO value.  0 for Saturation-based Speed "
					 "method, 1 for Standard Output Sensitivity method."));

static ConfigVariableDouble hdr_exposure_std_middle_grey
("hdr-exposure-std-middle-grey", 0.18,
 PRC_DESC("The middle grey value to use in the calculation of exposure "
          "using the Standard Output Sensitivity method."));

static ConfigVariableDouble hdr_luminance_adapation_rate
("hdr-luminance-adapation-rate", 1.0,
 PRC_DESC("Rate at which the average luminance is smoothly adjusted.  Higher "
          "is faster."));

IMPLEMENT_CLASS(HDREffect);
IMPLEMENT_CLASS( HDRPass );

HDRPass::HDRPass( PostProcess *pp ) :
	PostProcessPass( pp, "hdr" ),
	_quad_geom( nullptr ),
	_luminance(0.5f),
	_aperature(4.0f),
	_shutter_speed(1/60.0f),
	_iso(800.0f),
	_max_luminance(1.0f),
	_exposure(1.0f),
	_exposure_value(0.0f),
	_last_target_ev(0.0f)
{
	// This pass replaces the scene color pipe, so we need to use the same
	// color format.
	_fbprops.set_rgb_color(true);
	_fbprops.set_float_color(true);
	_fbprops.set_rgba_bits(16, 16, 16, 0);
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
	return std::max((float)hdr_min_ev.get_value(),
		std::min((float)hdr_max_ev.get_value(), std::log2(average_luminance * 100.0f / K)));
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
	//iso = clamp(compute_iso(aperature, shutter_speed, target_ev),
	//						(float)hdr_min_iso, (float)hdr_max_iso);
	iso = hdr_iso_value.get_value();

	// Figure out how far we were from the target exposure value.
	float ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);

	// Compute the final shutter speed.
	shutter_speed = std::clamp(shutter_speed * std::pow(2.0f, -ev_diff),
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
	//iso = clamp(compute_iso(aperature, shutter_speed, target_ev),
	//						(float)hdr_min_iso, (float)hdr_max_iso);
	iso = hdr_iso_value.get_value();

	// Figure out how far we were from the target exposure value.
	float ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);

	// Compute the final aperature.
	aperature = std::clamp(aperature * std::pow(std::sqrt(2.0f), ev_diff),
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
	//iso = clamp(compute_iso(aperature, shutter_speed, target_ev),
	//						(float)hdr_min_iso, (float)hdr_max_iso);
	iso = hdr_iso_value.get_value();

	// Apply half the difference in EV to the aperture.
	float ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);
	aperature = std::clamp(aperature * std::pow(std::sqrt(2.0f), ev_diff * 0.5f),
										(float)hdr_min_aperature, (float)hdr_max_aperature);

	// Apply the remaining difference to the shutter speed.
	ev_diff = target_ev - compute_ev(aperature, shutter_speed, iso);
	shutter_speed = std::clamp(shutter_speed * std::pow(2.0f, -ev_diff),
											  (float)hdr_min_shutter, (float)hdr_max_shutter);
}

/**
 * Get an exposure using the Saturation-based Speed method.
 */
float HDRPass::
get_saturation_based_exposure(float aperature, float shutter_speed, float iso) const {
	float l_max = (7800.0f / 65.0f) * std::pow(aperature, 2.0f) / (iso * shutter_speed);
	return 1.0f / l_max;
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

void HDRPass::update()
{
	PostProcessPass::update();

	if ( !hdr_auto_exposure ) {
		return;
	}

	PN_stdfloat ev_range = hdr_max_ev - hdr_min_ev;

	CPT(RenderAttrib) shattr;
	shattr = _histogram_compute_state->get_attrib(ShaderAttrib::get_class_slot());
	shattr = DCAST(ShaderAttrib, shattr)->set_shader_input("minLogLum_ooLogLumRange", LVecBase2(hdr_min_ev, 1.0f / ev_range));
	_histogram_compute_state = _histogram_compute_state->set_attrib(shattr);

	for (size_t i = 0; i < _luminance_buffers.size(); ++i) {
		shattr = _luminance_buffers[i]._compute_state->get_attrib(ShaderAttrib::get_class_slot());
		shattr = DCAST(ShaderAttrib, shattr)->set_shader_input("minLogLum_logLumRange", LVecBase2(hdr_min_ev, ev_range));
		_luminance_buffers[i]._compute_state = _luminance_buffers[i]._compute_state->set_attrib(shattr);
	}

	ClockObject *global_clock = ClockObject::get_global_clock();

	float target_ev;
	// Read in the luminance value computed by the shader.
	// Read from the texture that we are going to compute to next, which would be
	// the oldest un-read texture.
	_luminance_buffer_index = (_luminance_buffer_index + 1) % hdr_luminance_buffers;
	Texture *tex = _luminance_buffers[_luminance_buffer_index]._result_texture;
	if (tex->get_resident(_buffer->get_gsg()->get_prepared_objects())) {
		GraphicsEngine *engine = GraphicsEngine::get_global_ptr();
		engine->extract_texture_data(tex, _buffer->get_gsg());
		//engine->extract_texture_data(_histogram_buffer_texture, _buffer->get_gsg());

		//CPTA_uchar hist_image = _histogram_buffer_texture->get_ram_image();
		//std::cout << "Histogram data:\n";
		//for (int i = 0; i < hdr_num_buckets; i++) {
		//	unsigned int pixels = *(unsigned int *)(hist_image.v().data() + (i * 4));
		//	std::cout << "Bucket " << i << ": " << pixels << " pixels\n";
		//}

		CPTA_uchar image = tex->get_ram_image();
		target_ev = *(float *)(image.v().data());

		// Time average it.
		target_ev = _last_target_ev + (target_ev - _last_target_ev) *
			(1 - std::exp(-global_clock->get_dt() * hdr_luminance_adapation_rate));

		//target_ev = 1.0f;

	} else {
		target_ev = _last_target_ev;
	}

	_last_target_ev = target_ev;

	//std::cout << "Target ev: " << target_ev << "\n";

	_luminance = std::exp2(target_ev);

	// Clear out the current histogram for the next compute pass.
	//_histogram_buffer_texture->clear_image();

	NodePath camera_np = _pp->get_camera(0);
	Camera *camera = DCAST(Camera, camera_np.node());
	Lens *lens = camera->get_lens();

	// Now calculate the exposure.
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
	_exposure_value = std::exp2(_exposure);
	_aperature = aperature;
	_shutter_speed = shutter_speed;
	_iso = iso;

	//mat_motion_blur_strength = _shutter_speed * 2;

	// Apply the exposure scale to our pass's lens.
	lens = DCAST(Camera, get_camera().node())->get_lens();
	lens->set_exposure_scale(exposure);
}

void HDRPass::setup()
{
	PostProcessPass::setup();

	get_quad().set_shader(
		Shader::load(Shader::SL_GLSL,
			"shaders/postprocess/base.vert.glsl",
			"shaders/postprocess/apply_exposure.frag.glsl"));
	get_quad().set_shader_input("sceneTexture", _pp->get_output_pipe("scene_color"));

	PN_stdfloat ev_range = hdr_max_ev - hdr_min_ev;

	PT(Shader) histogram_shader = Shader::load_compute(
		Shader::SL_GLSL, "shaders/postprocess/build_histogram.compute.glsl");

	_histogram_buffer_texture = new Texture("hdr-histogram-buffer");
	_histogram_buffer_texture->setup_1d_texture(hdr_num_buckets, Texture::T_unsigned_int, Texture::F_r32i);
	_histogram_buffer_texture->set_clear_color(LColor(0.0f));

	CPT(RenderAttrib) histogram_shattr = ShaderAttrib::make(histogram_shader);
	histogram_shattr = DCAST(ShaderAttrib, histogram_shattr)->
		set_shader_input("sceneImage", _pp->get_output_pipe("scene_color"));
	histogram_shattr = DCAST(ShaderAttrib, histogram_shattr)->
		set_shader_input("histogram", _histogram_buffer_texture);
	histogram_shattr = DCAST(ShaderAttrib, histogram_shattr)->
		set_shader_input("minLogLum_ooLogLumRange", LVecBase2(hdr_min_ev, 1.0f / ev_range));
	_histogram_compute_state = RenderState::make(histogram_shattr);

	PT(Shader) calc_lum_shader = Shader::load_compute(
		Shader::SL_GLSL, "shaders/postprocess/calc_luminance.compute.glsl");
	CPT(RenderAttrib) lum_shattr = ShaderAttrib::make(calc_lum_shader);
	lum_shattr = DCAST(ShaderAttrib, lum_shattr)->
		set_shader_input("sceneImage", _pp->get_output_pipe("scene_color"));
	lum_shattr = DCAST(ShaderAttrib, lum_shattr)->
		set_shader_input("histogram", _histogram_buffer_texture);
	lum_shattr = DCAST(ShaderAttrib, lum_shattr)->
		set_shader_input("minLogLum_logLumRange", LVecBase2(hdr_min_ev, ev_range));

	_luminance_buffers.resize(hdr_luminance_buffers);
	for (int i = 0; i < hdr_luminance_buffers; i++) {
		PT(Texture) tex = new Texture("hdr-luminance-output");
		tex->setup_1d_texture(1, Texture::T_float, Texture::F_r32);
		tex->set_minfilter(SamplerState::FT_nearest);
		tex->set_magfilter(SamplerState::FT_nearest);
		CPT(RenderState) state = RenderState::make(
			DCAST(ShaderAttrib, lum_shattr)->
				set_shader_input("luminanceOutput", tex));
		_luminance_buffers[i]._compute_state = state;
		_luminance_buffers[i]._result_texture = tex;
	}
	_luminance_buffer_index = 0;
}

/**
 *
 */
void HDRPass::
on_draw(DisplayRegionDrawCallbackData *cbdata, GraphicsStateGuardian *gsg) {
	LVector2i dim = get_back_buffer_dimensions();

	// Clear out current histogram.
	_histogram_buffer_texture->clear_image();

	// Build the luminance histogram.
	gsg->set_state_and_transform(_histogram_compute_state,
															 TransformState::make_identity());
	gsg->dispatch_compute(dim[0] / hdr_work_group_size, dim[1] / hdr_work_group_size, 1);

	// Compute the luminance value.
	gsg->set_state_and_transform(_luminance_buffers[_luminance_buffer_index]._compute_state,
															 TransformState::make_identity());
	gsg->dispatch_compute(1, 1, 1);

	PostProcessPass::on_draw(cbdata, gsg);
}

HDREffect::HDREffect( PostProcess *pp ) :
	PostProcessEffect( pp, "hdr" )
{
	PT( HDRPass ) pass = new HDRPass( pp );
	pass->setup();
	// The output of this pass is the exposed scene.
	pass->add_color_output();
	// Override the scene color pipe with the exposed scene.
	pp->push_output_pipe("scene_color", pass->get_color_texture());

	add_pass( pass );
}

Texture *HDREffect::get_final_texture()
{
	// There's no color output for this effect.
	return nullptr;
}
