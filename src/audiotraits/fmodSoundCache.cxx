/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodSoundCache.cxx
 * @author brian
 * @date 2021-04-28
 */

#include "fmodSoundCache.h"
#include "config_fmodAudio.h"
#include "string_utils.h"
#include "fmodAudioManager.h"
#include "fmodAudioSound.h"
#include "virtualFileSystem.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "virtualFile.h"
#include "fmodAudioEngine.h"
#include "movieAudio.h"
#include "movieAudioCursor.h"
#include "config_putil.h"
#include "fmod_filesystem_hooks.h"

static PStatCollector cache_lookup_coll("App:FMOD:GetSound:CacheLookup");
static PStatCollector cache_miss_coll("App:FMOD:GetSound:CacheMiss");

/**
 *
 */
FMODSoundCache::
FMODSoundCache(FMODAudioEngine *engine) :
  _engine(engine)
{
}

/**
 * Returns a sound from the given file.
 */
PT(FMODSoundHandle) FMODSoundCache::
get_sound(const Filename &filename, bool positional, bool stream) {
  if (fmodAudio_cat.is_debug()) {
    fmodAudio_cat.debug()
      << "get_sound(): " << filename << "\n";
  }

  cache_lookup_coll.start();
  PT(FMODSoundHandle) &handle = _sounds[filename];
  cache_lookup_coll.stop();
  if (handle != nullptr) {
    return handle;
  }

  PStatTimer timer(cache_miss_coll);

  handle = _empty_sound;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename resolved = filename;
  if (!vfs->resolve_filename(resolved, get_model_path().get_value())) {
    return handle;
  }

  PT(VirtualFile) file = vfs->get_file(resolved);
  if (file == nullptr) {
    return handle;
  }

  FMOD::System *sys = _engine->get_system();

  if (fmodAudio_cat.is_debug()) {
    fmodAudio_cat.debug()
      << "Sound is not in cache; loading from disk\n";
  }

  fmodAudio_cat.info()
    << "Loading sound " << resolved << "\n";

  FMOD_RESULT result = FMOD_OK;
  FMOD::Sound *sound = nullptr;

  int flags = FMOD_DEFAULT;
#ifdef HAVE_STEAM_AUDIO
  bool use_steam_audio_positional = fmod_use_steam_audio;
#else
  bool use_steam_audio_positional = false;
#endif
  if (positional && !use_steam_audio_positional) {
    // We're using the built-in FMOD spatial audio system.
    flags |= FMOD_3D;
  } else {
    // If the sound is positional but we're using Steam Audio
    // for spatialization, indicate to FMOD that the sound is 2D.
    // We will bypass the built-in FMOD spatialization and rely
    // on Steam Audio to do it for us.
    flags |= FMOD_2D;
  }

  FMOD_CREATESOUNDEXINFO sound_info;
  memset(&sound_info, 0, sizeof(sound_info));
  sound_info.cbsize = sizeof(sound_info);

  bool is_mid = false;

  std::string ext = downcase(filename.get_extension());
  if (ext == "mid") {
    is_mid = true;
    // Get the MIDI parameters.
    const std::string &dls_name = _engine->get_dls_name();
    if (!dls_name.empty()) {
      sound_info.dlsname = dls_name.c_str();
      audio_debug("Using DLS file " << sound_info.dlsname);
    } else {
      sound_info.dlsname = nullptr;
    }
    // Need this flag so we can correctly query the length of MIDIs.
    flags |= FMOD_ACCURATETIME;
  } else if (ext == "mp3") {
    // Need this flag so we can correctly query the length of MP3s.
    flags |= FMOD_ACCURATETIME;
  }

  const char *name_or_data = filename.c_str();

  if (!stream) {
    // Pre-read the file right now, and pass it in as a memory buffer.  This
    // avoids threading issues completely, because all of the reading
    // happens right here.
    vector_uchar mem_buffer;
    file->read_file(mem_buffer, true);
    sound_info.length = mem_buffer.size();
    if (mem_buffer.size() != 0) {
      name_or_data = (const char *)&mem_buffer[0];
    }
    flags |= FMOD_OPENMEMORY;
    if (!is_mid) {
      if (fmod_compressed_samples) {
        flags |= FMOD_CREATECOMPRESSEDSAMPLE;
      } else {
        flags |= FMOD_CREATESAMPLE;
      }
    }
    if (fmodAudio_cat.is_debug()) {
      fmodAudio_cat.debug()
        << "Reading " << filename << " into memory (" << sound_info.length
        << " bytes)\n";
    }
    result =
      sys->createSound(name_or_data, flags, &sound_info, &sound);

  } else {
    // We want to stream the sound data from disk.

    result = FMOD_ERR_FILE_BAD;

    SubfileInfo info;
    if (file->get_system_info(info)) {
      // The file exists on disk (or it's part of a multifile that exists on
      // disk), so we can have FMod read the file directly.  This is also
      // safe, because FMod uses its own IO operations that don't involve
      // Panda, so this can safely happen in an FMod thread.
      std::string os_filename = info.get_filename().to_os_specific();
      name_or_data = os_filename.c_str();
      sound_info.fileoffset = (unsigned int)info.get_start();
      sound_info.length = (unsigned int)info.get_size();
      flags |= FMOD_CREATESTREAM;
      if (fmodAudio_cat.is_debug()) {
        fmodAudio_cat.debug()
          << "Streaming " << filename << " from disk (" << name_or_data
          << ", " << sound_info.fileoffset << ", " << sound_info.length << ")\n";
      }

      result =
        sys->createSound(name_or_data, flags, &sound_info, &sound);
    }

    // If FMOD can't directly read the file (eg. if Panda is locking it for
    // write, or it's compressed) we have to use the callback interface.
    if (result == FMOD_ERR_FILE_BAD) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
      // Otherwise, if the Panda threading system is compiled in, we can
      // assign callbacks to read the file through the VFS.
      name_or_data = (const char *)file.p();
      sound_info.fileoffset = 0;
      sound_info.length = (unsigned int)info.get_size();
      sound_info.fileuseropen = pfmod_open_callback;
      sound_info.fileuserdata = file.p();
      sound_info.fileuserclose = pfmod_close_callback;
      sound_info.fileuserread = pfmod_read_callback;
      sound_info.fileuserseek = pfmod_seek_callback;
      flags |= FMOD_CREATESTREAM;
      if (fmodAudio_cat.is_debug()) {
        fmodAudio_cat.debug()
          << "Streaming " << filename << " from disk using callbacks\n";
      }
      result =
        sys->createSound(name_or_data, flags, &sound_info, &sound);

#else  // HAVE_THREADS && !SIMPLE_THREADS
      // Without threads, we can't safely read this file.
      name_or_data = "";

      fmodAudio_cat.warning()
        << "Cannot stream " << filename << "; file is not literally on disk.\n";
#endif
    }
  }

  if (result != FMOD_OK) {
    audio_error("createSound(" << filename << "): " << FMOD_ErrorString(result));

  } else {
    // Some WAV files contain a loop bit.  This is not handled consistently.
    // Override it.
    sound->setLoopCount(1);
    sound->setMode(FMOD_LOOP_OFF);

    handle = new FMODSoundHandle(sound, resolved);
  }

  return handle;
}

/**
 *
 */
PT(FMODSoundHandle) FMODSoundCache::
get_sound(MovieAudio *audio, bool positional, bool stream) {
  // If the movie references a filename, check the filename against the
  // cache.
  Filename filename = audio->get_filename();
  if (!filename.empty()) {
    Sounds::const_iterator it = _sounds.find(filename);
    if (it != _sounds.end()) {
      // Here's the cache!
      return (*it).second;
    }
  }

  // The movie's filename is either not in cache or it doesn't originate
  // from a file (meaning it's dynamically generated audio, like voice data).

  PT(MovieAudioCursor) cur = audio->open();
  if (cur == nullptr) {
    // Couldn't open the cursor, return the empty sound.
    if (!filename.empty()) {
      _sounds[filename] = _empty_sound;
      return _empty_sound;
    }
  }

  // TODO: Support streaming MovieAudios.

  FMOD::System *sys = _engine->get_system();

  FMOD_RESULT result = FMOD_OK;
  FMOD::Sound *sound = nullptr;

  int flags = FMOD_DEFAULT;
#ifdef HAVE_STEAM_AUDIO
  bool use_steam_audio_positional = fmod_use_steam_audio;
#else
  bool use_steam_audio_positional = false;
#endif
  if (positional && !use_steam_audio_positional) {
    // We're using the built-in FMOD spatial audio system.
    flags |= FMOD_3D;
  } else {
    // If the sound is positional but we're using Steam Audio
    // for spatialization, indicate to FMOD that the sound is 2D.
    // We will bypass the built-in FMOD spatialization and rely
    // on Steam Audio to do it for us.
    flags |= FMOD_2D;
  }

  FMOD_CREATESOUNDEXINFO sound_info;
  memset(&sound_info, 0, sizeof(sound_info));
  sound_info.cbsize = sizeof(sound_info);

  // Pre-read the file right now, and pass it in as a memory buffer.  This
  // avoids threading issues completely, because all of the reading
  // happens right here.

  int num_samples = cur->audio_rate() * cur->audio_channels() * cur->length();
  int16_t *data = new int16_t[num_samples];
  cur->read_samples(cur->audio_rate() * cur->length(), data);
  sound_info.length = num_samples * sizeof(int16_t);
  sound_info.numchannels = cur->audio_channels();
  sound_info.defaultfrequency = cur->audio_rate();
  // MovieAudio uses 16-bit signed integer PCM.
  sound_info.format = FMOD_SOUND_FORMAT_PCM16;
  flags |= FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_OPENRAW;
  if (fmodAudio_cat.is_debug()) {
    fmodAudio_cat.debug()
      << "Reading " << filename << " into memory (" << sound_info.length
      << " bytes)\n";
  }
  result = sys->createSound((const char *)data, flags, &sound_info, &sound);
  PT(FMODSoundHandle) handle;
  if (!fmod_audio_errcheck("sys->createSound() (MovieAudio)", result)) {
    handle = _empty_sound;
  } else {
    // Some WAV files contain a loop bit.  This is not handled consistently.
    // Override it.
    sound->setLoopCount(1);
    sound->setMode(FMOD_LOOP_OFF);
    handle = new FMODSoundHandle(sound, filename);
  }
  delete[] data;

  if (!filename.empty()) {
    _sounds[filename] = handle;
  }
  return handle;
}

/**
 *
 */
void FMODSoundCache::
initialize() {
  // Cache off a silent sound to use when the actual sound can't be loaded
  // for whatever reason.

  FMOD_CREATESOUNDEXINFO sound_info;
  memset(&sound_info, 0, sizeof(sound_info));
  char blank_data[100];
  memset(blank_data, 0, sizeof(blank_data));
  sound_info.cbsize = sizeof(sound_info);
  sound_info.length = sizeof(blank_data);
  sound_info.numchannels = 1;
  sound_info.defaultfrequency = 8000;
  sound_info.format = FMOD_SOUND_FORMAT_PCM8;
  int flags = FMOD_OPENMEMORY | FMOD_OPENRAW | FMOD_CREATESAMPLE;

  FMOD::Sound *sound = nullptr;
  FMOD_RESULT result = _engine->get_system()->createSound(blank_data, flags, &sound_info, &sound);
  fmod_audio_errcheck("createSound (blank)", result);

  _empty_sound = new FMODSoundHandle(sound, "empty");
}
