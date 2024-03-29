/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioManager.h
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "config_audio.h"
#include "audioSound.h"
#include "luse.h"
#include "movieAudio.h"
#include "atomicAdjust.h"

typedef AudioManager *Create_AudioManager_proc(const std::string &, AudioManager *);

class DSP;
class RayTraceScene;

class EXPCL_PANDA_AUDIO AudioManager : public TypedReferenceCount {
PUBLISHED:

  enum SpeakerModeCategory {
    // These enumerants line up one-to-one with the FMOD SPEAKERMODE
    // enumerants.
    SPEAKERMODE_default,
    SPEAKERMODE_raw,
    SPEAKERMODE_mono,
    SPEAKERMODE_stereo,
    SPEAKERMODE_quad,
    SPEAKERMODE_surround,
    SPEAKERMODE_5point1,
    SPEAKERMODE_7point1,
    SPEAKERMODE_7point1point4,
    SPEAKERMODE_max,
    SPEAKERMODE_COUNT
  };

  enum SpeakerId {
    SPK_none = -1,
    SPK_front_left,
    SPK_front_right,
    SPK_front_center,
    SPK_sub,
    SPK_surround_left,
    SPK_surround_right,
    SPK_back_left,
    SPK_back_right,
    SPK_top_front_left,
    SPK_top_front_right,
    SPK_top_back_left,
    SPK_top_back_right,
    SPK_COUNT,
  };

  // DSP methods
  INLINE bool add_dsp_to_head(DSP *dsp);
  INLINE bool add_dsp_to_tail(DSP *dsp);
  virtual bool insert_dsp(int index, DSP *dsp);
  virtual bool remove_dsp(DSP *dsp);
  virtual void remove_all_dsps();
  virtual int get_num_dsps() const;

  // Create an AudioManager for each category of sounds you have.  E.g.
  // MySoundEffects = create_AudioManager::AudioManager(); MyMusicManager =
  // create_AudioManager::AudioManager(); ... my_sound =
  // MySoundEffects.get_sound("neatSfx.mp3"); my_music =
  // MyMusicManager.get_sound("introTheme.mid");

  virtual ~AudioManager();

  virtual void shutdown();

  // If you're interested in knowing whether this audio manager is valid,
  // here's the call to do it.  It is not necessary to check whether the audio
  // manager is valid before making other calls.  You are free to use an
  // invalid sound manager, you may get silent sounds from it though.  The
  // sound manager and the sounds it creates should not crash the application
  // even when the objects are not valid.
  virtual bool is_valid() = 0;

  // Get a sound:
  virtual PT(AudioSound) get_sound(const Filename &file_name, bool positional = false, bool stream = false) = 0;
  /**
   * Effectively returns a copy of the indicated AudioSound that can be
   * manipulated independently of each other.  In the FMOD implementation, this
   * shares the same sound data, but creates a different channel for playing.
   */
  virtual PT(AudioSound) get_sound(AudioSound *source) = 0;
  virtual PT(AudioSound) get_sound(MovieAudio *source, bool positional = false, bool stream = false) = 0;

  PT(AudioSound) get_null_sound();

  // Tell the AudioManager there is no need to keep this one cached.  This
  // doesn't break any connection between AudioSounds that have already given
  // by get_sound() from this manager.  It's only affecting whether the
  // AudioManager keeps a copy of the sound in its poolcache.
  virtual void uncache_sound(const Filename &file_name) = 0;
  virtual void clear_cache() = 0;
  virtual void set_cache_limit(unsigned int count) = 0;
  virtual unsigned int get_cache_limit() const = 0;

  // Control volume: FYI: If you start a sound with the volume off and turn
  // the volume up later, you'll hear the sound playing at that late point.  0
  // = minimum; 1.0 = maximum.  inits to 1.0.
  virtual void set_volume(PN_stdfloat volume) = 0;
  virtual PN_stdfloat get_volume() const = 0;

  // Turn the manager on or off.  If you play a sound while the manager is
  // inactive, it won't start.  If you deactivate the manager while sounds are
  // playing, they'll stop.  If you activate the manager while looping sounds
  // are playing (those that have a loop_count of zero), they will start
  // playing from the beginning of their loop.  Defaults to true.
  virtual void set_active(bool flag) = 0;
  virtual bool get_active() const = 0;

  // This controls the number of sounds that you allow at once.  This is more
  // of a user choice -- it avoids talk over and the creation of a cacophony.
  // It can also be used to help performance.  0 == unlimited.  1 == mutually
  // exclusive (one sound at a time).  Which is an example of: n == allow n
  // sounds to be playing at the same time.
  virtual void set_concurrent_sound_limit(unsigned int limit = 0) = 0;
  virtual unsigned int get_concurrent_sound_limit() const = 0;

  // This is likely to be a utility function for the concurrent_sound_limit
  // options.  It is exposed as an API, because it's reasonable that it may be
  // useful to be here.  It reduces the number of concurrently playing sounds
  // to count by some implementation specific means.  If the number of sounds
  // currently playing is at or below count then there is no effect.
  virtual void reduce_sounds_playing_to(unsigned int count) = 0;

  // Stop playback on all sounds managed by this manager.  This is effectively
  // the same as reduce_sounds_playing_to(0), but this call may be for
  // efficient on some implementations.
  virtual void stop_all_sounds() = 0;

  // This should be called every frame.  Failure to call could cause problems.
  virtual void update();

  static Filename get_dls_pathname();
  MAKE_PROPERTY(dls_pathname, get_dls_pathname);

  virtual void set_reverb(DSP *reverb_dsp);
  virtual void set_steam_audio_reverb();
  virtual void clear_reverb();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out) const;

public:
  static void register_AudioManager_creator(Create_AudioManager_proc* proc);

protected:
  friend class AudioSound;

  // Avoid adding data members (instance variables) to this mostly abstract
  // base class.  This allows implementors of various sound systems the best
  // flexibility.

  AtomicAdjust::Pointer _null_sound;

  AudioManager();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioManager",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

inline std::ostream &
operator << (std::ostream &out, const AudioManager &mgr) {
  mgr.output(out);
  return out;
}

#include "audioManager.I"

#endif /* AUDIOMANAGER_H */
