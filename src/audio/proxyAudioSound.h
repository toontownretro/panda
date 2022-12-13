/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file proxyAudioSound.h
 * @author brian
 * @date 2022-12-13
 */

#ifndef PROXYAUDIOSOUND_H
#define PROXYAUDIOSOUND_H

#include "pandabase.h"
#include "audioSound.h"
#include "steamAudioProperties.h"

/**
 * This is a proxy layer on top of an actual AudioSound instance, intended
 * for use with async sound loading.
 *
 * It implements the entire AudioSound interface, delegating the calls to the
 * actual underlying AudioSound, if there is one.
 *
 * If there is not an actual underlying AudioSound, the state of the sound
 * from calls by the user is tracked, and applied to the actual AudioSound
 * when it comes in.
 */
class EXPCL_PANDA_AUDIO ProxyAudioSound : public AudioSound {
  DECLARE_CLASS(ProxyAudioSound, AudioSound);

PUBLISHED:
  INLINE ProxyAudioSound();
  INLINE ProxyAudioSound(ProxyAudioSound *copy);

  INLINE void set_real_sound(AudioSound *sound);
  INLINE AudioSound *get_real_sound() const;
  MAKE_PROPERTY(real_sound, get_real_sound, set_real_sound);

  virtual void play() override;
  virtual void stop() override;

  virtual void set_loop(bool loop=true) override;
  virtual bool get_loop() const override;

  virtual void set_loop_count(unsigned long loop_count=1) override;
  virtual unsigned long get_loop_count() const override;

  virtual void set_loop_start(PN_stdfloat loop_start=0) override;
  virtual PN_stdfloat get_loop_start() const override;

  virtual void set_time(PN_stdfloat time=0.0) override;
  virtual PN_stdfloat get_time() const override;

  virtual void set_volume(PN_stdfloat volume=1.0) override;
  virtual PN_stdfloat get_volume() const override;

  virtual void set_balance(PN_stdfloat balance=0.0) override;
  virtual PN_stdfloat get_balance() const override;

  virtual void set_play_rate(PN_stdfloat rate=1.0f) override;
  virtual PN_stdfloat get_play_rate() const override;

  virtual void set_active(bool flag=true) override;
  virtual bool get_active() const override;

  virtual void set_finished_event(const std::string &event) override;
  virtual const std::string &get_finished_event() const override;

  virtual const std::string &get_name() const override;

  virtual PN_stdfloat length() const override;

  virtual void set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) override;
  virtual LPoint3 get_3d_position() const override;
  virtual LQuaternion get_3d_quat() const override;
  virtual LVector3 get_3d_velocity() const override;

  virtual void set_3d_min_distance(PN_stdfloat dist) override;
  virtual PN_stdfloat get_3d_min_distance() const override;

  virtual void apply_steam_audio_properties(const SteamAudioProperties &props) override;

  virtual void set_loop_range(PN_stdfloat start, PN_stdfloat end = -1.0f) override;

  virtual SoundStatus status() const override;

public:
  void apply_state_to_real_sound();

private:
  PT(AudioSound) _real;

  PN_stdfloat _time;
  PN_stdfloat _play_rate;
  PN_stdfloat _loop_start;
  PN_stdfloat _loop_end;
  PN_stdfloat _volume;

  bool _got_balance;
  PN_stdfloat _balance;

  bool _active;

  bool _got_steam_audio_props;
  SteamAudioProperties _sprops;

  SoundStatus _status;

  int _loop_count;

  std::string _finished_event;

  std::string _name;

  PN_stdfloat _3d_min_distance;

  LPoint3 _pos;
  LQuaternion _quat;
  LVector3 _vel;
};

#include "proxyAudioSound.I"

#endif // PROXYAUDIOSOUND_H
