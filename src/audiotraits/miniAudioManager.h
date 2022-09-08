/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file miniAudioManager.h
 * @author brian
 * @date 2022-09-06
 */

#ifndef MINIAUDIOMANAGER_H
#define MINIAUDIOMANAGER_H

#include "pandabase.h"
#include "audioManager.h"
#include "miniaudio.h"

/**
 *
 */
class MiniAudioManager : public AudioManager {
  DECLARE_CLASS(MiniAudioManager, AudioManager);

public:
  MiniAudioManager();
  ~MiniAudioManager();

  virtual PT(AudioSound) get_sound(const Filename &filename, bool positional, int mode) override;
  virtual PT(AudioSound) get_sound(AudioSound *source) override;
  virtual PT(AudioSound) get_sound(MovieAudio *source, bool positional, int mode) override;

  virtual void uncache_sound(const Filename &filename) override;
  virtual void clear_cache() override;
  virtual void set_cache_limit(unsigned int count) override;
  virtual unsigned int get_cache_limit() const override;

  virtual void set_volume(PN_stdfloat volume) override;
  virtual PN_stdfloat get_volume() const override;

  virtual bool is_valid() override;

  virtual void set_active(bool flag) override;
  virtual bool get_active() const override;

  virtual void set_concurrent_sound_limit(unsigned int limit) override;
  virtual unsigned int get_concurrent_sound_limit() const override;
  virtual void reduce_sounds_playing_to(unsigned int count) override;

  virtual void stop_all_sounds() override;

  static bool initialize_ma();

private:
  // Governs whether miniaudio has been globally initialized.
  static bool _ma_initialized;
  static ma_vfs_callbacks *_ma_vfs;
  static ma_engine *_ma_engine;
  static ma_device *_ma_playback_device;
  static ma_resource_manager *_ma_rsrc_mgr;

  ma_sound_group *_sound_group;

  friend class MiniAudioSound;
};

AudioManager *Create_MiniAudioManager();

#include "miniAudioManager.I"

#endif // MINIAUDIOMANAGER_H
