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

extern EXPCL_PANDA_POSTPROCESS ConfigVariableBool hdr_auto_exposure;

struct hdrbucket_t
{
	float luminance_min;
	float luminance_max;

	int pixels;

	PT( OcclusionQueryContext ) ctx;
};

static const int HDR_NUM_BUCKETS = 16;

class EXPCL_PANDA_POSTPROCESS HDRPass : public PostProcessPass
{
	DECLARE_CLASS( HDRPass, PostProcessPass );

PUBLISHED:
	HDRPass( PostProcess *pp );

	virtual void setup_quad();
	virtual void setup_region();

	virtual void setup();

	virtual void update();

public:
	void draw( CallbackData *data );

private:
	float find_location_of_percent_bright_pixels(
		float percent_bright_pixels, float same_bin_snap,
		int total_pixel_count );

private:
	PTA_stdfloat _luminance_min_max;
	CPT( Geom ) _hdr_quad_geom;
	CPT( RenderState ) _hdr_geom_state;

	// Calculated exposure level based on histogram
	float _exposure;

	int _current_bucket;
	hdrbucket_t _buckets[HDR_NUM_BUCKETS];
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

#endif // HDR_H
