/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodSoundCache.h
 * @author brian
 * @date 2021-04-28
 */

#ifndef FMODSOUNDCACHE_H
#define FMODSOUNDCACHE_H

#include "pandabase.h"
#include "pmap.h"
#include "filename.h"
#include "referenceCount.h"
#include "pointerTo.h"

#include <fmod.hpp>

class FMODAudioEngine;
class VirtualFile;
class MovieAudio;

/**
 * Handle to an FMOD::Sound object that releases the sound when the last
 * reference to the handle goes away.
 */
class EXPCL_FMOD_AUDIO FMODSoundHandle : public ReferenceCount {
public:
  INLINE FMODSoundHandle(FMOD::Sound *sound, const Filename &filename, bool positional) :
    _sound(sound),
    _orig_filename(filename),
    _positional(positional) { }

  INLINE ~FMODSoundHandle() { if (_sound != nullptr) { _sound->release(); _sound = nullptr; } }
  INLINE FMOD::Sound *get_sound() const { return _sound; }
  INLINE const Filename &get_orig_filename() const { return _orig_filename; }
  INLINE bool is_positional() const { return _positional; }

private:
  FMOD::Sound *_sound;
  Filename _orig_filename;

  bool _positional;
};

/**
 * This class manages the loading of FMOD::Sound objects (which contain the
 * actual sound data), so we can share them between multiple FMODAudioSounds
 * that reference the same filename.  Each FMODAudioSound just creates a
 * different channel to play the same sound.
 */
class EXPCL_FMOD_AUDIO FMODSoundCache : public ReferenceCount {
public:
  FMODSoundCache(FMODAudioEngine *engine);

  PT(FMODSoundHandle) get_sound(const Filename &filename, bool positional, bool stream);
  PT(FMODSoundHandle) get_sound(MovieAudio *audio, bool positional, bool stream);

  void initialize();

  INLINE void clear_sounds();

private:
  static FMOD_RESULT F_CALLBACK
  open_callback(const char *name, unsigned int *file_size,
                void **handle, void *user_data);

  static FMOD_RESULT F_CALLBACK
  close_callback(void *handle, void *user_data);

  static FMOD_RESULT F_CALLBACK
  read_callback(void *handle, void *buffer, unsigned int size_bytes,
                unsigned int *bytes_read, void *user_data);

  static FMOD_RESULT F_CALLBACK
  seek_callback(void *handle, unsigned int pos, void *user_data);

private:
  typedef pflat_hash_map<Filename, PT(FMODSoundHandle)> Sounds;
  Sounds _sounds;

  PT(FMODSoundHandle) _empty_sound;

  FMODAudioEngine *_engine;
};

#include "fmodSoundCache.I"

#endif // FMODSOUNDCACHE_H
