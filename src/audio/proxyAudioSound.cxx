/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file proxyAudioSound.cxx
 * @author brian
 * @date 2022-12-13
 */

#include "proxyAudioSound.h"

IMPLEMENT_CLASS(ProxyAudioSound);

/**
 *
 */
void ProxyAudioSound::
play() {
  if (_real != nullptr) {
    _real->play();
  } else {
    _status = PLAYING;
  }
}

/**
 *
 */
void ProxyAudioSound::
stop() {
  if (_real != nullptr) {
    _real->stop();
  } else {
    _status = READY;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_loop(bool loop) {
  if (_real != nullptr) {
    _real->set_loop(loop);
  } else {
    _loop_count = loop ? 0 : 1;
  }
}

/**
 *
 */
bool ProxyAudioSound::
get_loop() const {
  if (_real != nullptr) {
    return _real->get_loop();
  } else {
    return _loop_count == 0;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_loop_count(unsigned long count) {
  if (_real != nullptr) {
    _real->set_loop_count(count);
  } else {
    _loop_count = count;
  }
}

/**
 *
 */
unsigned long ProxyAudioSound::
get_loop_count() const {
  if (_real != nullptr) {
    return _real->get_loop_count();
  } else {
    return _loop_count;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_loop_start(PN_stdfloat start) {
  if (_real != nullptr) {
    _real->set_loop_start(start);
  } else {
    _loop_start = start;
  }
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
get_loop_start() const {
  if (_real != nullptr) {
    return _real->get_loop_start();
  } else {
    return _loop_start;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_time(PN_stdfloat time) {
  if (_real != nullptr) {
    _real->set_time(time);
  } else {
    _time = time;
  }
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
get_time() const {
  if (_real != nullptr) {
    return _real->get_time();
  } else {
    return _time;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_volume(PN_stdfloat volume) {
  if (_real != nullptr) {
    _real->set_volume(volume);
  } else {
    _volume = volume;
  }
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
get_volume() const {
  if (_real != nullptr) {
    return _real->get_volume();
  } else {
    return _volume;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_balance(PN_stdfloat balance) {
  if (_real != nullptr) {
    _real->set_balance(balance);
  } else {
    _got_balance = true;
    _balance = balance;
  }
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
get_balance() const {
  if (_real != nullptr) {
    return _real->get_balance();
  } else {
    return _balance;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_play_rate(PN_stdfloat rate) {
  if (_real != nullptr) {
    _real->set_play_rate(rate);
  } else {
    _play_rate = rate;
  }
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
get_play_rate() const {
  if (_real != nullptr) {
    return _real->get_play_rate();
  } else {
    return _play_rate;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_active(bool active) {
  if (_real != nullptr) {
    _real->set_active(active);
  } else {
    _active = active;
  }
}

/**
 *
 */
bool ProxyAudioSound::
get_active() const {
  if (_real != nullptr) {
    return _real->get_active();
  } else {
    return _active;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_finished_event(const std::string &event) {
  if (_real != nullptr) {
    _real->set_finished_event(event);
  } else {
    _finished_event = event;
  }
}

/**
 *
 */
const std::string &ProxyAudioSound::
get_finished_event() const {
  if (_real != nullptr) {
    return _real->get_finished_event();
  } else {
    return _finished_event;
  }
}

/**
 *
 */
const std::string &ProxyAudioSound::
get_name() const {
  return _name;
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
length() const {
  if (_real != nullptr) {
    return _real->length();
  } else {
    // Arbitrary.
    return 1.0f;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_3d_attributes(const LPoint3 &pos, const LQuaternion &quat, const LVector3 &vel) {
  if (_real != nullptr) {
    _real->set_3d_attributes(pos, quat, vel);
  } else {
    _pos = pos;
    _quat = quat;
    _vel = vel;
  }
}

/**
 *
 */
LPoint3 ProxyAudioSound::
get_3d_position() const {
  if (_real != nullptr) {
    return _real->get_3d_position();
  } else {
    return _pos;
  }
}

/**
 *
 */
LQuaternion ProxyAudioSound::
get_3d_quat() const {
  if (_real != nullptr) {
    return _real->get_3d_quat();
  } else {
    return _quat;
  }
}

/**
 *
 */
LVector3 ProxyAudioSound::
get_3d_velocity() const {
  if (_real != nullptr) {
    return _real->get_3d_velocity();
  } else {
    return _vel;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  if (_real != nullptr) {
    _real->set_3d_min_distance(dist);
  } else {
    _3d_min_distance = dist;
  }
}

/**
 *
 */
PN_stdfloat ProxyAudioSound::
get_3d_min_distance() const {
  if (_real != nullptr) {
    return _real->get_3d_min_distance();
  } else {
    return _3d_min_distance;
  }
}

/**
 *
 */
void ProxyAudioSound::
apply_steam_audio_properties(const SteamAudioProperties &props) {
  if (_real != nullptr) {
    _real->apply_steam_audio_properties(props);
  } else {
    _got_steam_audio_props = true;
    _sprops = props;
  }
}

/**
 *
 */
void ProxyAudioSound::
set_loop_range(PN_stdfloat start, PN_stdfloat end) {
  if (_real != nullptr) {
    _real->set_loop_range(start, end);
  } else {
    _loop_start = start;
    _loop_end = end;
  }
}

/**
 *
 */
AudioSound::SoundStatus ProxyAudioSound::
status() const {
  if (_real != nullptr) {
    return _real->status();
  } else {
    return _status;
  }
}

/**
 *
 */
void ProxyAudioSound::
apply_state_to_real_sound() {
  nassertv(_real != nullptr);

  _real->set_loop_range(_loop_start, _loop_end);
  _real->set_loop_count(_loop_count);
  _real->set_play_rate(_play_rate);
  _real->set_volume(_volume);
  _real->set_time(_time);
  _real->set_active(_active);
  _real->set_finished_event(_finished_event);
  if (_got_steam_audio_props) {
    _real->apply_steam_audio_properties(_sprops);
  }
  _real->set_3d_min_distance(_3d_min_distance);
  _real->set_3d_attributes(_pos, _quat, _vel);
  if (_got_balance) {
    _real->set_balance(_balance);
  }
  if (_status == PLAYING) {
    _real->play();
  } else {
    _real->stop();
  }

  // Change the name to reflect that the proxy has a real sound.
  _name = "proxy-" + _real->get_name();
}
