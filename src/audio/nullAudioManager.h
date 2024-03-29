/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nullAudioManager.h
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#ifndef NULLAUDIOMANAGER_H
#define NULLAUDIOMANAGER_H

#include "audioManager.h"
#include "nullAudioSound.h"

class EXPCL_PANDA_AUDIO NullAudioManager : public AudioManager {
  // All of these methods are stubbed out to some degree.  If you're looking
  // for a starting place for a new AudioManager, please consider looking at
  // the openalAudioManager.

public:
  NullAudioManager();
  virtual ~NullAudioManager();

  virtual bool is_valid();

  virtual PT(AudioSound) get_sound(const Filename &, bool positional = false, bool stream = false);
  virtual PT(AudioSound) get_sound(MovieAudio *sound, bool positional = false, bool stream = false);
  virtual PT(AudioSound) get_sound(AudioSound *sound) override;

  virtual void uncache_sound(const Filename &);
  virtual void clear_cache();
  virtual void set_cache_limit(unsigned int);
  virtual unsigned int get_cache_limit() const;

  virtual void set_volume(PN_stdfloat);
  virtual PN_stdfloat get_volume() const;

  virtual void set_play_rate(PN_stdfloat);
  virtual PN_stdfloat get_play_rate() const;

  virtual void set_active(bool);
  virtual bool get_active() const;

  virtual void set_concurrent_sound_limit(unsigned int limit);
  virtual unsigned int get_concurrent_sound_limit() const;

  virtual void reduce_sounds_playing_to(unsigned int count);

  virtual void stop_all_sounds();


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AudioManager::init_type();
    register_type(_type_handle, "NullAudioManager",
                  AudioManager::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif /* NULLAUDIOMANAGER_H */
