/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioManager.cxx
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#include "config_audio.h"
#include "audioManager.h"
#include "atomicAdjust.h"
#include "nullAudioManager.h"
#include "windowsRegistry.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "load_dso.h"

#ifdef _WIN32
#include <windows.h>  // For GetSystemDirectory()
#endif

using std::string;

TypeHandle AudioManager::_type_handle;

/**
 *
 */
AudioManager::
~AudioManager() {
  if (_null_sound != nullptr) {
    unref_delete((AudioSound *)_null_sound);
  }
}

/**
 *
 */
AudioManager::
AudioManager() {
  _null_sound = nullptr;
}

/**
 * Call this at exit time to shut down the audio system.  This will invalidate
 * all currently-active AudioManagers and AudioSounds in the system.  If you
 * change your mind and want to play sounds again, you will have to recreate
 * all of these objects.
 */
void AudioManager::
shutdown() {
}

/**
 * Returns a special NullAudioSound object that has all the interface of a
 * normal sound object, but plays no sound.  This same object may also be
 * returned by get_sound() if it fails.
 */
PT(AudioSound) AudioManager::
get_null_sound() {
  if (_null_sound == nullptr) {
    AudioSound *new_sound = new NullAudioSound;
    new_sound->ref();
    void *result = AtomicAdjust::compare_and_exchange_ptr(_null_sound, nullptr, (void *)new_sound);
    if (result != nullptr) {
      // Someone else must have assigned the AudioSound first.  OK.
      nassertr(_null_sound != new_sound, nullptr);
      unref_delete(new_sound);
    }
    nassertr(_null_sound != nullptr, nullptr);
  }

  return (AudioSound *)_null_sound;
}

/**
 * Inserts the specified DSP filter into the DSP chain at the specified index.
 * Returns true if the DSP filter is supported by the audio implementation,
 * false otherwise.
 */
bool AudioManager::
insert_dsp(int index, DSP *dsp) {
  // Must be implemented by audio implementation.
  return false;
}

/**
 * Removes the specified DSP filter from the DSP chain. Returns true if the
 * filter was in the DSP chain and was removed, false otherwise.
 */
bool AudioManager::
remove_dsp(DSP *dsp) {
  // Must be implemented by audio implementation.
  return false;
}

/**
 * Removes all DSP filters from the DSP chain.
 */
void AudioManager::
remove_all_dsps() {
  // Must be implemented in audio implementation.
}

/**
 * Returns the number of DSP filters present in the DSP chain.
 */
int AudioManager::
get_num_dsps() const {
  return 0;
}

/**
 * Must be called every frame.  Failure to call this every frame could cause
 * problems for some audio managers.
 */
void AudioManager::
update() {
  // Intentionally blank.
}

/**
 * Returns the full pathname to the DLS file, as specified by the Config.prc
 * file, or the default for the current OS if appropriate.  Returns empty
 * string if the DLS file is unavailable.
 */
Filename AudioManager::
get_dls_pathname() {
  Filename dls_filename = audio_dls_file;
  if (!dls_filename.empty()) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(dls_filename, get_model_path());

    return dls_filename;
  }

#ifdef _WIN32
  Filename pathname;

  // Get the registry key from DirectMusic
  string os_filename = WindowsRegistry::get_string_value("SOFTWARE\\Microsoft\\DirectMusic", "GMFilePath", "");

  if (!os_filename.empty()) {
    pathname = Filename::from_os_specific(os_filename);
  } else {
    char sysdir[MAX_PATH+1];
    GetSystemDirectory(sysdir,MAX_PATH+1);
    pathname = Filename(Filename::from_os_specific(sysdir), Filename("drivers/gm.dls"));
  }
  pathname.make_true_case();
  return pathname;

#elif defined(IS_OSX)
  // This appears to be the standard place for this file on OSX 10.4.
  return Filename("/System/Library/Components/CoreAudio.component/Contents/Resources/gs_instruments.dls");

#else
  return Filename();
#endif
}

/**
 *
 */
void AudioManager::
output(std::ostream &out) const {
  out << get_type();
}

/**
 *
 */
void AudioManager::
write(std::ostream &out) const {
  out << (*this) << "\n";
}

/**
 *
 */
void AudioManager::
set_reverb(DSP *reverb_dsp) {
}

/**
 *
 */
void AudioManager::
set_steam_audio_reverb() {
}

/**
 *
 */
void AudioManager::
clear_reverb() {
}
