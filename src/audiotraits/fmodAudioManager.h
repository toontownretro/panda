/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioManager.h
 * @author cort
 * @date 2003-01-22
 * @author ben
 * @date 2003-10-22
 * Prior system by: cary
 * @author Stan Rosenbaum "Staque" - Spring 2006
 * @author lachbr
 * @date 2020-10-04
 *
 * Hello, all future Panda audio code people! This is my errata
 * documentation to help any future programmer maintain FMOD and PANDA.
 *
 * This documentation more then that is needed, but I wanted to go all
 * out, with the documentation. Because I was a totally newbie at
 * programming [especially with C/C++] this semester I want to make
 * sure future code maintainers have that insight I did not when
 * starting on the PANDA project here at the ETC/CMU.
 *
 * As of Spring 2006, Panda's FMOD audio support has been pretty much
 * completely rewritten. This has been done so PANDA can use FMOD-EX
 * [or AKA FMOD 4] and some of its new features.
 *
 * First, the FMOD-EX API itself has been completely rewritten compared
 * to previous versions. FMOD now handles any type of audio files, wave
 * audio [WAV, AIF, MP3, OGG, etc...] or musical file [MID, TRACKERS]
 * as the same type of an object. The API has also been structured more
 * like a sound studio, with 'sounds' and 'channels'. This will be
 * covered more in the FMODAudioSound.h/.cxx sources.
 *
 * Second, FMOD now offers virtually unlimited sounds to be played at
 * once via their virtual channels system. Actually the theoretical
 * limit is around 4000, but that is still a lot. What you need to know
 * about this, is that even thought you might only hear 32 sound being
 * played at once, FMOD will keep playing any additional sounds, and
 * swap those on virtual channels in and out with those on real
 * channels depending on priority, or distance [if you are dealing with
 * 3D audio].
 *
 * Third, FMOD's DSP support has been added. So you can now add GLOBAL
 * or SOUND specific DSP effects. Right not you can only use FMOD's
 * built in DSP effects.  But adding support for FMOD's support of VST
 * effects shouldn't be that hard.
 *
 * As for the FmodManager itself, it is pretty straight forward, I
 * hope. As a manager class, it will create the FMOD system with the
 * "_system" variable which is an instance of FMOD::SYSTEM. (Actually,
 * we create only one global _system variable now, and share it with
 * all outstanding FmodManager objects--this appears to be the way FMOD
 * wants to work.)  The FmodManager class is also the one responsible
 * for creation of Sounds, DSP, and maintaining the GLOBAL DSP chains
 * [The GLOBAL DSP chain is the DSP Chain which affects ALL the
 * sounds].
 *
 * Any way that is it for an intro, lets move on to looking at the rest
 * of the code.
 */

#ifndef FMODAUDIOMANAGER_H
#define FMODAUDIOMANAGER_H

// First the includes.
#include "pandabase.h"
#include "pset.h"
#include "pdeque.h"

#include "audioManager.h"
#include "dsp.h"
#include "updateSeq.h"
#include "thread.h"
#include "pmutex.h"
#include "reMutex.h"

// The includes needed for FMOD
#include <fmod.hpp>
#include <fmod_errors.h>

class FMODAudioSound;
class FMODAudioEngine;

class EXPCL_FMOD_AUDIO FMODAudioManager : public AudioManager {
  friend class FMODAudioSound;
  friend class FMODSoundCache;

public:
  FMODAudioManager(const std::string &name, FMODAudioManager *parent, FMODAudioEngine *engine);
  virtual ~FMODAudioManager();

  virtual bool insert_dsp(int index, DSP *dsp);
  virtual bool remove_dsp(DSP *dsp);
  virtual void remove_all_dsps();
  virtual int get_num_dsps() const;

  virtual bool is_valid();

  virtual PT(AudioSound) get_sound(const Filename &filename, bool positional = false, bool stream = false);
  virtual PT(AudioSound) get_sound(AudioSound *source) override;
  virtual PT(AudioSound) get_sound(MovieAudio *source, bool positional = false, bool stream = false);

  virtual void set_volume(PN_stdfloat);
  virtual PN_stdfloat get_volume() const;

  virtual void set_active(bool);
  virtual bool get_active() const;

  virtual void stop_all_sounds();

  virtual void update();

  virtual void set_concurrent_sound_limit(unsigned int limit = 0);
  virtual unsigned int get_concurrent_sound_limit() const;
  virtual void reduce_sounds_playing_to(unsigned int count);

  // These are currently unused by the FMOD implementation.  Could possibly
  // implement them, though.
  virtual void uncache_sound(const Filename &);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int count);
  virtual unsigned int get_cache_limit() const;

  FMOD::DSP *get_fmod_dsp(DSP *panda_dsp) const;

private:
  void starting_sound(FMODAudioSound *sound);
  void stopping_sound(FMODAudioSound *sound);
  // Tell the manager that the sound dtor was called.
  void release_sound(FMODAudioSound* sound);

  void update_sounds();

private:
  // This global lock protects all access to FMod library interfaces.
  static ReMutex _lock;

public:
  FMOD::ChannelGroup *_channelgroup;

  bool _is_valid;
  bool _active;

  // Keeps track of sounds currently playing on the manager.
  // We hold a reference in this list to support fire-and-forget sounds.
  typedef pflat_hash_set<PT(FMODAudioSound)> SoundsPlaying;
  SoundsPlaying _sounds_playing;

  // All sounds created through this manager, playing or not.
  typedef pflat_hash_set<FMODAudioSound *, pointer_hash> AllSounds;
  AllSounds _all_sounds;

  // Mapping of Panda DSP instance to FMOD DSP instance.
  typedef phash_map<PT(DSP), FMOD::DSP *> FMODDSPs;
  FMODDSPs _dsps;

  unsigned int _concurrent_sound_limit;

  FMODAudioEngine *_engine;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "FMODAudioManager", AudioManager::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif /* FMODAUDIOMANAGER_H */
