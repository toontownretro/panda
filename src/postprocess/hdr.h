/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hdr.h
 * @author lachbr
 * @date 2019-07-22
 */

#ifndef HDR_H
#define HDR_H

#include "postProcessPass.h"
#include "postProcessEffect.h"

#include "geom.h"
#include "renderState.h"
#include "callbackObject.h"
#include "occlusionQueryContext.h"
#include "pta_float.h"
#include "configVariableBool.h"
#include "pvector.h"

extern EXPCL_PANDA_POSTPROCESS ConfigVariableBool hdr_auto_exposure;

class hdrbucket_t
{
public:
	float luminance_min;
	float luminance_max;

	int pixels;

	PT(OcclusionQueryContext) ctx;

	CPT(RenderState) state;
};

extern EXPCL_PANDA_POSTPROCESS ConfigVariableInt hdr_num_buckets;
extern EXPCL_PANDA_POSTPROCESS ConfigVariableInt hdr_num_ldr_buckets;

class EXPCL_PANDA_POSTPROCESS HDRPass : public PostProcessPass
{
	DECLARE_CLASS( HDRPass, PostProcessPass );

PUBLISHED:
	enum AutoExposureMethod {
		AEM_program_auto,
		AEM_shutter_priority,
		AEM_aperature_priority,
	};

	enum ExposureMethod {
		EM_saturation_speed,
		EM_standard_output,
	};

	HDRPass( PostProcess *pp );

	virtual void setup_quad();
	virtual void setup_region();

	virtual void setup();

	virtual void update();

	INLINE float get_luminance() const;
	INLINE float get_aperature() const;
	INLINE float get_shutter_speed() const;
	INLINE float get_iso() const;
	INLINE float get_max_luminance() const;
	INLINE float get_exposure() const;

public:
	void draw( CallbackData *data );

private:
	float find_location_of_percent_bright_pixels(
		float percent_bright_pixels, float same_bin_snap,
		int total_pixel_count );

	float get_average_histogram_luminance(int total_pixel_count) const;

	float compute_iso(float aperature, float shutter_speed, float iso) const;
	float compute_ev(float aperature, float shutter_speed, float iso) const;
	float compute_target_ev(float average_luminance) const;

	void apply_aperature_priority(float focal_length, float target_ev,
																float &aperature, float &shutter_speed,
																float &iso);
	void apply_shutter_priority(float focal_length, float target_ev,
															float &aperature, float &shutter_speed,
															float &iso);
	void apply_program_auto(float focal_length, float target_ev,
													float &aperature, float &shutter_speed,
													float &iso);

	float get_saturation_based_exposure(float aperature, float shutter_speed,
	                                    float iso) const;
	float get_standard_output_based_exposure(float aperature,
																					 float shutter_speed,
	                                    		 float iso, float middle_grey) const;

private:
	CPT(Geom) _quad_geom;

	// Calculated luminance based on histogram
	float _luminance;

	// Camera settings.
	float _aperature;
	float _shutter_speed;
	float _iso;
	float _max_luminance;
	float _exposure;

	int _current_bucket;
	pvector<hdrbucket_t> _buckets;
};

class EXPCL_PANDA_POSTPROCESS HDREffect : public PostProcessEffect
{
PUBLISHED:
	HDREffect( PostProcess *pp );

	virtual Texture *get_final_texture();

	INLINE HDRPass *get_hdr_pass() const
	{
		return (HDRPass *)_passes.get_data( 0 ).p();
	}
};

#include "hdr.I"

#endif // HDR_H
