/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file proxyAudioSound.I
 * @author brian
 * @date 2022-12-13
 */

/**
 * Initializes a ProxyAudioSound with no assigned real AudioSound.  The proxy
 * will track the state from user calls until a real AudioSound is assigned.
 */
INLINE ProxyAudioSound::
ProxyAudioSound() :
  _real(nullptr),
  _got_balance(false),
  _got_steam_audio_props(false),
  _time(0.0f),
  _play_rate(1.0f),
  _loop_start(0.0f),
  _loop_end(-1.0f),
  _volume(1.0f),
  _balance(0.0f),
  _active(true),
  _status(READY),
  _loop_count(1),
  _finished_event(""),
  _name("proxy"),
  _3d_min_distance(1.0f),
  _pos(0.0f),
  _quat(LQuaternion::ident_quat()),
  _vel(0.0f)
{
}

/**
 *
 */
INLINE ProxyAudioSound::
ProxyAudioSound(ProxyAudioSound *copy) :
  _real(copy->_real),
  _got_balance(copy->_got_balance),
  _got_steam_audio_props(false),
  _time(0.0f),
  _play_rate(copy->_play_rate),
  _loop_start(copy->_loop_start),
  _loop_end(copy->_loop_end),
  _volume(copy->_volume),
  _balance(copy->_balance),
  _3d_min_distance(copy->_3d_min_distance),
  _loop_count(1),
  _status(READY),
  _pos(0.0f),
  _quat(LQuaternion::ident_quat()),
  _vel(0.0f),
  _name("proxy"),
  _finished_event(""),
  _active(true)
{
}

/**
 *
 */
INLINE void ProxyAudioSound::
set_real_sound(AudioSound *sound) {
  nassertv(_real == nullptr);
  _real = sound;
  apply_state_to_real_sound();
}

/**
 *
 */
INLINE AudioSound *ProxyAudioSound::
get_real_sound() const {
  return _real;
}
