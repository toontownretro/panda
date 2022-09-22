/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_fmodAudio.h
 * @author cort
 */

#ifndef CONFIG_FMODAUDIO_H
#define CONFIG_FMODAUDIO_H

#include "pandabase.h"

#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "audioManager.h"

#include <fmod.hpp>

ConfigureDecl(config_fmodAudio, EXPCL_FMOD_AUDIO, EXPTP_FMOD_AUDIO);
NotifyCategoryDecl(fmodAudio, EXPCL_FMOD_AUDIO, EXPTP_FMOD_AUDIO);

extern ConfigVariableInt fmod_audio_preload_threshold;
extern ConfigVariableBool fmod_compressed_samples;
extern ConfigVariableBool fmod_debug;
extern ConfigVariableBool fmod_profile;
extern ConfigVariableInt fmod_dsp_buffer_size;
extern ConfigVariableInt fmod_dsp_buffer_count;

#ifdef HAVE_STEAM_AUDIO
extern ConfigVariableBool fmod_use_steam_audio;
#endif

extern bool _fmod_audio_errcheck(const char *context, FMOD_RESULT n);

#ifdef NDEBUG
#define fmod_audio_errcheck(context, n) (n == FMOD_OK)
#else
#define fmod_audio_errcheck(context, n) _fmod_audio_errcheck(context, n)
#endif // NDEBUG

// calculate gain based on atmospheric attenuation.
// as gain excedes threshold, round off (compress) towards 1.0 using spline
#define SND_GAIN_COMP_EXP_MAX	2.5f	// Increasing SND_GAIN_COMP_EXP_MAX fits compression curve more closely
                    // to original gain curve as it approaches 1.0.
#define SND_GAIN_COMP_EXP_MIN	0.8f

#define SND_GAIN_COMP_THRESH	0.5f		// gain value above which gain curve is rounded to approach 1.0

#define SND_DB_MAX				140.0	// max db of any sound source
#define SND_DB_MED				90.0	// db at which compression curve changes
#define SND_DB_MIN				60.0	// min db of any sound source

#define SND_GAIN_MAX 1
#define SND_GAIN_MIN 0.01

#define SND_REFDB 60.0
#define SND_REFDIST 36.0

#define SNDLVL_TO_DIST_MULT( sndlvl ) ( sndlvl ? ((pow( 10.0f, SND_REFDB / 20 ) / pow( 10.0f, (float)sndlvl / 20 )) / SND_REFDIST) : 0 )
#define DIST_MULT_TO_SNDLVL( dist_mult ) (int)( dist_mult ? ( 20 * log10( pow( 10.0f, SND_REFDB / 20 ) / (dist_mult * SND_REFDIST) ) ) : 0 )

extern EXPCL_FMOD_AUDIO void init_libfmod_audio();

#endif // CONFIG_FMODAUDIO_H
