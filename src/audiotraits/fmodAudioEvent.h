/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioEvent.h
 * @author brian
 * @date 2023-06-09
 */

#ifndef FMODAUDIOEVENT_H
#define FMODAUDIOEVENT_H

#include "pandabase.h"
#include "audioSound.h"

#include <fmod_studio.hpp>

class FMODAudioEngine;

/**
 * Represents an instanced event from FMOD Studio created by the sound
 * designer, rather than a manually loaded sound in code.  It has the
 * same interface as a regular AudioSound to be compatible with existing
 * code, but some methods don't do anything.  Events are also not associated
 * with any AudioManager, but the AudioEngine directly.  The sound designer
 * controls the channel groups in FMOD Studio.
 */
class FMODAudioEvent : public AudioSound {
public:
  FMODAudioEvent(FMODAudioEngine *engine, FMOD::Studio::EventDescription *desc, FMOD::Studio::EventInstance *event);
  virtual ~FMODAudioEvent();

  virtual void set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) override;
  virtual LPoint3 get_3d_position() const override;
  virtual LQuaternion get_3d_quat() const override;
  virtual LVector3 get_3d_velocity() const override;

  virtual void set_play_rate(PN_stdfloat rate) override;
  virtual PN_stdfloat get_play_rate() const override;

  virtual void set_time(PN_stdfloat start_time = 0.0f) override;
  virtual PN_stdfloat get_time() const override;

  virtual void set_volume(PN_stdfloat volume = 1.0f) override;
  virtual PN_stdfloat get_volume() const override;

  virtual void set_active(bool flag = true) override;
  virtual bool get_active() const override;

  virtual void set_loop(bool loop = true) override;
  virtual bool get_loop() const override;

  virtual void set_loop_count(unsigned long count = 1) override;
  virtual unsigned long get_loop_count() const override;

  virtual void set_loop_start(PN_stdfloat start = 0.0f) override;
  virtual PN_stdfloat get_loop_start() const override;

  virtual void set_balance(PN_stdfloat balance = 0.0f) override;
  virtual PN_stdfloat get_balance() const override;

  virtual void set_finished_event(const std::string &event) override;
  virtual const std::string &get_finished_event() const override;

  virtual PN_stdfloat length() const override;

  virtual SoundStatus status() const override;

  virtual void play() override;
  virtual void stop() override;

  virtual const std::string &get_name() const override;

  INLINE FMOD::Studio::EventDescription *get_event_description() const;
  INLINE FMOD::Studio::EventInstance *get_event() const;

private:
  FMOD::Studio::EventDescription *_event_desc;
  FMOD::Studio::EventInstance *_event;

  FMODAudioEngine *_engine;

  std::string _name;
  std::string _finished_event;

  LPoint3 _pos;
  LVector3 _vel;
  LQuaternion _quat;
};

#include "fmodAudioEvent.I"

#endif // FMODAUDIOEVENT_H
