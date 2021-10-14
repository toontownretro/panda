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

static PStatCollector cache_lookup_coll("App:FMOD:GetSound:CacheLookup");
static PStatCollector cache_miss_coll("App:FMOD:GetSound:CacheMiss");

FMODSoundCache *FMODSoundCache::_ptr = nullptr;

/**
 * Returns a sound from the given file.
 */
PT(FMODSoundHandle) FMODSoundCache::
get_sound(FMODAudioManager *mgr, VirtualFile *file, bool positional) {
  Filename filename = file->get_filename();

  if (fmodAudio_cat.is_debug()) {
    fmodAudio_cat.debug()
      << "get_sound(): " << filename << "\n";
  }

  cache_lookup_coll.start();
  Sounds::const_iterator it = _sounds.find(filename);
  cache_lookup_coll.stop();
  if (it != _sounds.end()) {
    if (fmodAudio_cat.is_debug()) {
      fmodAudio_cat.debug()
        << "Sound is in cache; reusing\n";
    }
    return (*it).second;
  }

  PStatTimer timer(cache_miss_coll);

  if (fmodAudio_cat.is_debug()) {
    fmodAudio_cat.debug()
      << "Sound is not in cache; loading from disk\n";
  }

  FMOD_RESULT result = FMOD_OK;
  FMOD::Sound *sound = nullptr;

  bool preload = (fmod_audio_preload_threshold < 0) || (file->get_file_size() < fmod_audio_preload_threshold);
  int flags = FMOD_DEFAULT;
  flags |= positional ? FMOD_3D : FMOD_2D;

  FMOD_CREATESOUNDEXINFO sound_info;
  memset(&sound_info, 0, sizeof(sound_info));
  sound_info.cbsize = sizeof(sound_info);

  std::string ext = downcase(filename.get_extension());
  if (ext == "mid") {
    // Get the MIDI parameters.
    memcpy(&sound_info, &mgr->_midi_info, sizeof(sound_info));
    if (sound_info.dlsname != nullptr) {
      audio_debug("Using DLS file " << sound_info.dlsname);
    }
    // Need this flag so we can correctly query the length of MIDIs.
    flags |= FMOD_ACCURATETIME;
  } else if (ext == "mp3") {
    // Need this flag so we can correctly query the length of MP3s.
    flags |= FMOD_ACCURATETIME;
  }

  const char *name_or_data = filename.c_str();
  std::string os_filename;

  vector_uchar mem_buffer;
  SubfileInfo info;
  if (preload) {
    // Pre-read the file right now, and pass it in as a memory buffer.  This
    // avoids threading issues completely, because all of the reading
    // happens right here.
    file->read_file(mem_buffer, true);
    sound_info.length = mem_buffer.size();
    if (mem_buffer.size() != 0) {
      name_or_data = (const char *)&mem_buffer[0];
    }
    flags |= FMOD_OPENMEMORY;
    if (fmodAudio_cat.is_debug()) {
      fmodAudio_cat.debug()
        << "Reading " << filename << " into memory (" << sound_info.length
        << " bytes)\n";
    }
    result =
      mgr->_system->createSound(name_or_data, flags, &sound_info, &sound);
  }
  else {
    result = FMOD_ERR_FILE_BAD;

    if (file->get_system_info(info)) {
      // The file exists on disk (or it's part of a multifile that exists on
      // disk), so we can have FMod read the file directly.  This is also
      // safe, because FMod uses its own IO operations that don't involve
      // Panda, so this can safely happen in an FMod thread.
      os_filename = info.get_filename().to_os_specific();
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
        mgr->_system->createSound(name_or_data, flags, &sound_info, &sound);
    }

    // If FMOD can't directly read the file (eg. if Panda is locking it for
    // write, or it's compressed) we have to use the callback interface.
    if (result == FMOD_ERR_FILE_BAD) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
      // Otherwise, if the Panda threading system is compiled in, we can
      // assign callbacks to read the file through the VFS.
      name_or_data = (const char *)file;
      sound_info.fileoffset = 0;
      sound_info.length = (unsigned int)info.get_size();
      sound_info.fileuseropen = open_callback;
      sound_info.fileuserdata = file;
      sound_info.fileuserclose = close_callback;
      sound_info.fileuserread = read_callback;
      sound_info.fileuserseek = seek_callback;
      flags |= FMOD_CREATESTREAM;
      if (fmodAudio_cat.is_debug()) {
        fmodAudio_cat.debug()
          << "Streaming " << filename << " from disk using callbacks\n";
      }
      result =
        mgr->_system->createSound(name_or_data, flags, &sound_info, &sound);

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

    // We couldn't load the sound file.  Create a blank sound record instead.
    FMOD_CREATESOUNDEXINFO sound_info;
    memset(&sound_info, 0, sizeof(sound_info));
    char blank_data[100];
    memset(blank_data, 0, sizeof(blank_data));
    sound_info.cbsize = sizeof(sound_info);
    sound_info.length = sizeof(blank_data);
    sound_info.numchannels = 1;
    sound_info.defaultfrequency = 8000;
    sound_info.format = FMOD_SOUND_FORMAT_PCM16;
    int flags = FMOD_OPENMEMORY | FMOD_OPENRAW;

    result = mgr->_system->createSound(blank_data, flags, &sound_info, &sound);
    fmod_audio_errcheck("createSound (blank)", result);
  }

  // Some WAV files contain a loop bit.  This is not handled consistently.
  // Override it.
  sound->setLoopCount(1);
  sound->setMode(FMOD_LOOP_OFF);

  PT(FMODSoundHandle) handle = new FMODSoundHandle(sound);
  _sounds[filename] = handle;

  return handle;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODSoundCache::
open_callback(const char *name, unsigned int *file_size,
              void **handle, void *user_data) {
  // We actually pass in the VirtualFile pointer as the "name".
  VirtualFile *file = (VirtualFile *)name;
  if (file == nullptr) {
    return FMOD_ERR_FILE_NOTFOUND;
  }
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "open_callback(" << *file << ")\n";
  }

  std::istream *str = file->open_read_file(true);

  (*file_size) = file->get_file_size(str);
  (*handle) = (void *)str;

  // Explicitly ref the VirtualFile since we're storing it in a void pointer
  // instead of a PT(VirtualFile).
  file->ref();

  return FMOD_OK;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODSoundCache::
close_callback(void *handle, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "close_callback(" << *file << ")\n";
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  std::istream *str = (std::istream *)handle;
  vfs->close_read_file(str);

  // Explicitly unref the VirtualFile pointer.
  unref_delete(file);

  return FMOD_OK;
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODSoundCache::
read_callback(void *handle, void *buffer, unsigned int size_bytes,
              unsigned int *bytes_read, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "read_callback(" << *file << ", " << size_bytes << ")\n";
  }

  std::istream *str = (std::istream *)handle;
  str->read((char *)buffer, size_bytes);
  (*bytes_read) = str->gcount();

  // We can't yield here, since this callback is made within a sub-thread--an
  // OS-level sub-thread spawned by FMod, not a Panda thread.  But we will
  // only execute this code in the true-threads case anyway.
  // thread_consider_yield();

  if (str->eof()) {
    if ((*bytes_read) == 0) {
      return FMOD_ERR_FILE_EOF;
    } else {
      // Report the EOF next time.
      return FMOD_OK;
    }
  } if (str->fail()) {
    return FMOD_ERR_FILE_BAD;
  } else {
    return FMOD_OK;
  }
}

/**
 * A hook into Panda's virtual file system.
 */
FMOD_RESULT F_CALLBACK FMODSoundCache::
seek_callback(void *handle, unsigned int pos, void *user_data) {
  VirtualFile *file = (VirtualFile *)user_data;
  if (fmodAudio_cat.is_spam()) {
    fmodAudio_cat.spam()
      << "seek_callback(" << *file << ", " << pos << ")\n";
  }

  std::istream *str = (std::istream *)handle;
  str->clear();
  str->seekg(pos);

  if (str->fail() && !str->eof()) {
    return FMOD_ERR_FILE_COULDNOTSEEK;
  } else {
    return FMOD_OK;
  }
}
