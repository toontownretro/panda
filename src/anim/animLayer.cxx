/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animLayer.cxx
 * @author brian
 * @date 2021-05-24
 */

#include "animLayer.h"
#include "clockObject.h"
#include "animSequencePlayer.h"

/**
 *
 */
AnimLayer::
AnimLayer() {
  init(nullptr);
}

/**
 *
 */
void AnimLayer::
init(AnimSequencePlayer *owner) {
  _player = owner;
  _flags = 0;
  _weight = 0;
  _cycle = 0;
  _prev_cycle = 0;
  _sequence_finished = false;
  _looping = false;
  _activity = -1;
  _sequence = 0;
  _sequence_parity = 0;
  _priority = 0;
  _order = max_anim_layers;
  _kill_rate = 100.0f;
  _kill_delay = 0.0f;
  _play_rate = 1.0f;
  _last_access = ClockObject::get_global_clock()->get_frame_time();
  _layer_anim_time = 0;
  _layer_fade_out_time = 0;
}

/**
 *
 */
bool AnimLayer::
is_abandoned() const {
  ClockObject *global_clock = ClockObject::get_global_clock();
  return (is_active() && !is_autokill() && !is_killme() &&
          _last_access > 0.0f && (global_clock->get_frame_time() - _last_access > 0.2f));
}

/**
 *
 */
void AnimLayer::
mark_active() {
  _last_access = ClockObject::get_global_clock()->get_frame_time();
}

/**
 *
 */
void AnimLayer::
advance(PN_stdfloat interval, AnimSequencePlayer *owner) {
  PN_stdfloat cycle_rate = owner->get_sequence_cycle_rate(_sequence);

  _prev_cycle = _cycle;
  _cycle += interval * cycle_rate * _play_rate;

  if (_cycle < 0.0f) {
    if (_looping) {
      _cycle -= (int)_cycle;
    } else {
      _cycle = 0.0f;
    }
  } else if (_cycle >= 1.0f) {
    _sequence_finished = true;
    if (_looping) {
      _cycle -= (int)_cycle;
    } else {
      _cycle = 1.0f;
    }
  }

  if (is_autoramp()) {
    _weight = 1;

    // Blend in?
    if (_blend_in != 0.0f) {
      if (_cycle < _blend_in) {
        _weight = _cycle / _blend_in;
      }
    }

    // Blend out?
    if (_blend_out != 0.0f) {
      if (_cycle > 1.0f - _blend_out) {
        _weight = (1.0f - _cycle) / _blend_out;
      }
    }

    _weight = 3.0 * _weight * _weight - 2.0 * _weight * _weight * _weight;
    if (_sequence == 0) {
      _weight = 0;
    }
  }
}
