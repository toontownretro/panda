/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file miniAudioSound.h
 * @author brian
 * @date 2022-09-06
 */

#ifndef MINIAUDIOSOUND_H
#define MINIAUDIOSOUND_H

#include "pandabase.h"
#include "audioSound.h"
#include "miniaudio.h"
#include "audioManager.h"

class VirtualFile;
class MiniAudioManager;

/**
 *
 */
class MiniAudioSound : public AudioSound {
  DECLARE_CLASS(MiniAudioSound, AudioSound);

public:
  MiniAudioSound(VirtualFile *file, bool positional, MiniAudioManager *mgr, AudioManager::StreamMode mode);
  MiniAudioSound(MiniAudioSound *other, MiniAudioManager *mgr);
  ~MiniAudioSound();

  virtual void play() override;
  virtual void stop() override;

  virtual void set_time(PN_stdfloat time = 0.0f) override;
  virtual PN_stdfloat get_time() const override;

  virtual void set_volume(PN_stdfloat volume) override;
  virtual PN_stdfloat get_volume() const override;

  virtual void set_balance(PN_stdfloat balance = 0.0f) override;
  virtual PN_stdfloat get_balance() const override;

  virtual void set_play_rate(PN_stdfloat play_rate) override;
  virtual PN_stdfloat get_play_rate() const override;

  virtual void set_loop(bool loop = true) override;
  virtual bool get_loop() const override;

  virtual void set_loop_range(PN_stdfloat start, PN_stdfloat end = -1.0f) override;

  virtual void set_loop_count(unsigned long count) override { }
  virtual unsigned long get_loop_count() const override { return 0; }

  virtual void set_active(bool flag = true) override;
  virtual bool get_active() const override;

  virtual void set_finished_event(const std::string &event) override;
  virtual const std::string &get_finished_event() const override;

  virtual const std::string &get_name() const override;

  virtual PN_stdfloat length() const override;

  virtual SoundStatus status() const override;

private:
  ma_sound *_sound;
  std::string _finished_event;
  std::string _name;
};

#include "miniAudioSound.I"

#endif // MINIAUDIOSOUND_H
