/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_miniaudio.cxx
 * @author brian
 * @date 2022-09-06
 */

#include "config_miniaudio.h"
#include "pandaSystem.h"
#include "miniAudioManager.h"
#include "miniAudioSound.h"

ConfigureDef(config_miniaudio);
ConfigureFn(config_miniaudio) {
  init_libpminiaudio();
}

NotifyCategoryDef(miniaudio, ":audio");

ConfigVariableBool miniaudio_load_and_decode
 ("miniaudio-load-and-decode", true,
  PRC_DESC("When true, miniaudio will decode audio files into raw PCM "
           "at load time, rather than during audio mixing.  Trades slightly "
           "longer load times for more free time on the audio thread."));
ConfigVariableBool miniaudio_decode_to_device_format
 ("miniaudio-decode-to-device-format", true,
  PRC_DESC("When true, miniaudio will convert an audio file's decoded PCM "
           "data to the format of the playback device on load.  This only "
           "applies when miniaudio-load-and-decode is true."));
ConfigVariableInt miniaudio_preload_threshold
 ("miniaudio-preload-threshold", 250000, // default is a quarter megabyte
  PRC_DESC("Specifies the maximum number of bytes an audio file may be for "
           "it to be preloaded entirely into memory.  Otherwise, the audio "
           "file will be streamed directly from disk.  Set to 0 to stream "
           "every audio file, -1 (or an unrealistically high value) to always "
           "preload."));
ConfigVariableInt miniaudio_sample_rate
 ("miniaudio-sample-rate", 44100);
ConfigVariableInt miniaudio_num_channels
 ("miniaudio-num-channels", 2);

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpminiaudio() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  MiniAudioManager::init_type();
  MiniAudioSound::init_type();

  AudioManager::register_AudioManager_creator(&Create_MiniAudioManager);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("miniaudio");
  ps->add_system("audio");
  ps->set_system_tag("audio", "implementation", "miniaudio");
}

/**
 * This function is called when the dynamic library is loaded; it should
 * return the Create_AudioManager function appropriate to create a
 * MiniAudioManager.
 */
Create_AudioManager_proc *
get_audio_manager_func_pminiaudio() {
  init_libpminiaudio();
  return &Create_MiniAudioManager;
}
