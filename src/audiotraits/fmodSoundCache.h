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
#include "virtualFile.h"

#include "fmod.hpp"

class FMODAudioManager;

/**
 * Handle to an FMOD::Sound object that releases the sound when the last
 * reference to the handle goes away.
 */
class EXPCL_FMOD_AUDIO FMODSoundHandle : public ReferenceCount {
public:
  INLINE FMODSoundHandle(FMOD::Sound *sound) : _sound(sound) { }
  INLINE ~FMODSoundHandle() { if (_sound != nullptr) { _sound->release(); _sound = nullptr; } }
  INLINE FMOD::Sound *get_sound() const { return _sound; }

private:
  FMOD::Sound *_sound;
};

/**
 * This class manages the loading of FMOD::Sound objects (which contain the
 * actual sound data), so we can share them between multiple FMODAudioSounds
 * that reference the same filename.  Each FMODAudioSound just creates a
 * different channel to play the same sound.
 */
class EXPCL_FMOD_AUDIO FMODSoundCache {
public:
  PT(FMODSoundHandle) get_sound(FMODAudioManager *mgr, VirtualFile *file, bool positional);

  INLINE static FMODSoundCache *get_global_ptr();

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
  static FMODSoundCache *_ptr;

  typedef pmap<Filename, PT(FMODSoundHandle)> Sounds;
  Sounds _sounds;
};

#include "fmodSoundCache.I"

#endif // FMODSOUNDCACHE_H
