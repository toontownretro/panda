/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fmodAudioEvent.cxx
 * @author brian
 * @date 2023-06-09
 */

#include "fmodAudioEvent.h"
#include "fmodAudioEngine.h"

extern FMOD_VECTOR lvec_to_fmod(const LVecBase3 &vec);

/**
 *
 */
FMODAudioEvent::
FMODAudioEvent(FMODAudioEngine *engine, FMOD::Studio::EventDescription *desc, FMOD::Studio::EventInstance *event) :
  _event_desc(desc),
  _event(event),
  _engine(engine),
  _finished_event("")
{
  char path[256];
  int length;
  _event_desc->getPath(path, 256, &length);
  _name = std::string(path, length);
}

/**
 *
 */
FMODAudioEvent::
~FMODAudioEvent() {
  if (_engine != nullptr) {
    _engine->release_event(this);
  }
  if (_event != nullptr) {
    _event->release();
    _event = nullptr;
  }
}

/**
 * Changes the play rate of the event.  This affects the speed that time
 * progresses as well as the pitch of sounds associated with the event.
 */
void FMODAudioEvent::
set_play_rate(PN_stdfloat rate) {
  _event->setPitch(rate);
}

/**
 *
 */
PN_stdfloat FMODAudioEvent::
get_play_rate() const {
  PN_stdfloat pitch;
  _event->getPitch(&pitch);
  return pitch;
}

/**
 * Sets the current time along the event's timeline in seconds.
 */
void FMODAudioEvent::
set_time(PN_stdfloat time) {
  // FMOD uses milliseconds.
  int ms = (int)(time * 1000.0f);
  _event->setTimelinePosition(ms);
}

/**
 * Returns the current time along the event's timeline in seconds.
 */
PN_stdfloat FMODAudioEvent::
get_time() const {
  int ms;
  _event->getTimelinePosition(&ms);
  return (PN_stdfloat)ms / 1000.0f;
}

/**
 *
 */
void FMODAudioEvent::
set_volume(PN_stdfloat volume) {
  _event->setVolume(volume);
}

/**
 *
 */
PN_stdfloat FMODAudioEvent::
get_volume() const {
  float vol;
  _event->getVolume(&vol);
  return (PN_stdfloat)vol;
}

/**
 *
 */
void FMODAudioEvent::
set_active(bool flag) {
}

/**
 *
 */
bool FMODAudioEvent::
get_active() const {
  return true;
}

/**
 *
 */
void FMODAudioEvent::
set_loop(bool flag) {
}

/**
 *
 */
bool FMODAudioEvent::
get_loop() const {
  return false;
}

/**
 *
 */
void FMODAudioEvent::
set_loop_count(unsigned long count) {
}

/**
 *
 */
unsigned long FMODAudioEvent::
get_loop_count() const {
  return 1u;
}

/**
 *
 */
void FMODAudioEvent::
set_loop_start(PN_stdfloat start) {
}

/**
 *
 */
PN_stdfloat FMODAudioEvent::
get_loop_start() const {
  return 0.0f;
}

/**
 *
 */
void FMODAudioEvent::
set_balance(PN_stdfloat balance) {
}

/**
 *
 */
PN_stdfloat FMODAudioEvent::
get_balance() const {
  return 0.0f;
}

/**
 *
 */
void FMODAudioEvent::
set_finished_event(const std::string &event) {
  _finished_event = event;
}

/**
 *
 */
const std::string &FMODAudioEvent::
get_finished_event() const {
  return _finished_event;
}

/**
 * Returns the length of the event in seconds.
 */
PN_stdfloat FMODAudioEvent::
length() const {
  int ms;
  _event_desc->getLength(&ms);
  return (PN_stdfloat)ms / 1000.0f;
}

/**
 *
 */
AudioSound::SoundStatus FMODAudioEvent::
status() const {
  if (!_event_desc->isValid() || !_event->isValid()) {
    return BAD;
  }

  FMOD_STUDIO_PLAYBACK_STATE state;
  _event->getPlaybackState(&state);

  switch (state) {
  case FMOD_STUDIO_PLAYBACK_STARTING:
  case FMOD_STUDIO_PLAYBACK_PLAYING:
  case FMOD_STUDIO_PLAYBACK_SUSTAINING:
  case FMOD_STUDIO_PLAYBACK_STOPPING:
    return PLAYING;

  default:
    return READY;
  }
}

/**
 *
 */
void FMODAudioEvent::
play() {
  _event->start();

  _engine->starting_event(this);
}

/**
 *
 */
void FMODAudioEvent::
stop() {
  _event->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}

/**
 *
 */
const std::string &FMODAudioEvent::
get_name() const {
  return _name;
}

/**
 *
 */
void FMODAudioEvent::
set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) {
  PN_stdfloat unit_scale = _engine->get_3d_unit_scale();

  // Game units to meters.
  _pos = pos / unit_scale;
  _vel = vel / unit_scale;
  _quat = quat;

  FMOD_3D_ATTRIBUTES attr;
  attr.forward = lvec_to_fmod(_quat.get_forward());
  attr.position = lvec_to_fmod(_pos);
  attr.up = lvec_to_fmod(_quat.get_up());
  attr.velocity = lvec_to_fmod(_vel);
  _event->set3DAttributes(&attr);
}

/**
 *
 */
LPoint3 FMODAudioEvent::
get_3d_position() const {
  return _pos * _engine->get_3d_unit_scale();
}

/**
 *
 */
LQuaternion FMODAudioEvent::
get_3d_quat() const {
  return _quat;
}

/**
 *
 */
LVector3 FMODAudioEvent::
get_3d_velocity() const {
  return _vel * _engine->get_3d_unit_scale();
}
