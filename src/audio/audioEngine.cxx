/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioEngine.cxx
 * @author brian
 * @date 2022-09-20
 */

#include "audioEngine.h"
#include "config_audio.h"
#include "load_dso.h"
#include "config_putil.h"
#include "nullAudioEngine.h"

IMPLEMENT_CLASS(AudioEngine);

AudioEngineProxy *AudioEngine::_engine_proxy = nullptr;

/**
 *
 */
PT(AudioEngine) AudioEngine::
make_engine() {
  PT(AudioEngine) engine;

  if (_engine_proxy == nullptr) {
    // No engine proxy yet, so load the requested audio library.  It should
    // register its proxy in the library's static init function.

    static bool loaded_audio_lib = false;

    if (!loaded_audio_lib) {
      loaded_audio_lib = true;

      if (!audio_library_name.empty() && audio_library_name != "null") {
        Filename dl_name = Filename::dso_filename("lib" + audio_library_name.get_value() + ".so");
        dl_name.to_os_specific();
        audio_debug("  dl_name=\"" << dl_name << "\"");
        void *handle = load_dso(get_plugin_path().get_value(), dl_name);
        if (handle == nullptr) {
          audio_error("  load_dso(" << dl_name << ") failed, will use NullAudioManager");
          audio_error("    "<<load_dso_error());
          nassertr(_engine_proxy == nullptr, nullptr);
        }

        // The library should've now registered its proxy.
        if (_engine_proxy == nullptr) {
          audio_error("Audio library " << audio_library_name << " did not register an AudioEngineProxy, cannot create an AudioEngine from it");
        }
      }
    }
  }

  if (_engine_proxy != nullptr) {
    // Have the proxy make us an engine.
    engine = _engine_proxy->make_engine();
    if (!engine->initialize()) {
      audio_error("Failed to initialize" << engine->get_type() << ", will use NullAudioEngine");
      engine = nullptr;
    }
  }

  if (engine == nullptr) {
    engine = new NullAudioEngine;
  }

  return engine;
}

/**
 *
 */
void AudioEngine::
register_engine_proxy(AudioEngineProxy *proxy) {
  _engine_proxy = proxy;
}

/**
 *
 */
void AudioEngine::
set_tracer(AudioTracer *tracer) {
}

/**
 *
 */
void AudioEngine::
clear_tracer() {
}
