/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_fmodAudio.cxx
 * @author cort
 */

#include "pandabase.h"

#include "config_fmodAudio.h"
#include "audioManager.h"
#include "fmodAudioEngine.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "pandaSystem.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_FMOD_AUDIO)
  #error Buildsystem error: BUILDING_FMOD_AUDIO not defined
#endif

ConfigureDef(config_fmodAudio);
NotifyCategoryDef(fmodAudio, ":audio");

ConfigureFn(config_fmodAudio) {
  init_libfmod_audio();
}

ConfigVariableInt fmod_audio_preload_threshold
  ("fmod-audio-preload-threshold", 1048576,
  PRC_DESC("Files that are smaller "
            "than this number of bytes will be preloaded and kept "
            "resident in memory, while files that are this size or larger "
            "will be streamed from disk.  Set this to -1 to preload "
            "every file."));

ConfigVariableBool fmod_debug
  ("fmod-debug", false,
   PRC_DESC("Set true to enable debug mode within FMOD internally.  Makes FMOD "
            "send logging messages to our Notify category.  Requires linking "
            "with the libfmodL library, instead of the regular libfmod."));

ConfigVariableBool fmod_profile
  ("fmod-profile", false,
   PRC_DESC("Set true to enable FMOD profiling.  Allows connecting to the "
            "application via the FMOD profiling tool to visualize the DSP "
            "graph, CPU and memory usage, etc."));

ConfigVariableInt fmod_dsp_buffer_size
  ("fmod-dsp-buffer-size", 1024,
   PRC_DESC("Sets the size of the audio buffer used by FMOD's software mixer.  "
            "A smaller buffer results in less latency, but can result in "
            "audio dropouts if the mixer cannot process the audio in the "
            "window of time provided by the buffer size."));

ConfigVariableInt fmod_dsp_buffer_count
  ("fmod-dsp-buffer-count", 4,
   PRC_DESC("Sets the number of audio buffers used by FMOD's software mixer.  "
            "Used in conjunction with fmod-dsp-buffer-size to control mixer "
            "latency."));

ConfigVariableBool fmod_compressed_samples
  ("fmod-compressed-samples", false,
  PRC_DESC("Setting this true allows FMOD to play compressed audio samples "
           "directly from memory without having to decompress and decode to "
           "raw PCM at load time.  Trades CPU usage for less memory taken up "
           "by compressed audio samples."));

#ifdef HAVE_STEAM_AUDIO
ConfigVariableBool fmod_use_steam_audio
  ("fmod-use-steam-audio", false,
  PRC_DESC("If true, indicates that Steam Audio should be used for simulation "
            "and spatialization of positional sounds.  This is only available "
            "if Steam Audio support has been compiled in."));
#endif


/**
 * Central dispatcher for audio errors.
 */
bool
_fmod_audio_errcheck(const char *context, FMOD_RESULT result) {
  if (result != FMOD_OK) {
    fmodAudio_cat.error() <<
      context << ": " << FMOD_ErrorString(result) << "\n";
    return false;
  }
  return true;
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libfmod_audio() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;
  FMODAudioEngine::init_type();
  FMODAudioManager::init_type();
  FMODAudioSound::init_type();

  AudioEngine::register_engine_proxy(new FMODAudioEngineProxy);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("FMOD");
  ps->add_system("audio");
  ps->set_system_tag("audio", "implementation", "FMOD");
}
