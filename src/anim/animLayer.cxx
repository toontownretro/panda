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
#include "animChannel.h"
#include "character.h"

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
init(Character *character) {
  _character = character;
  _play_mode = PM_none;
  _start_cycle = 0.0f;
  _play_cycles = 0.0f;
  _last_advance_time = 0.0f;
  _flags = 0;
  _weight = 0;
  _kill_weight = 0.0f;
  _ramp_weight = 0.0f;
  _cycle = 0;
  _prev_cycle = 0;
  _sequence_finished = false;
  _activity = -1;
  _sequence = 0;
  _sequence_parity = 0;
  _prev_sequence_parity = 0;
  _priority = 0;
  _order = 0;
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
 * Returns true if the layer is currently playing an animation, or false
 * if the animation finished playing or there is no animation assigned to the
 * layer.
 */
bool AnimLayer::
is_playing() const {
  if (_sequence < 0) {
    return false;
  }

  if (_play_rate == 0.0f) {
    return false;
  }

  switch (_play_mode) {
  case PM_pose:
  case PM_none:
  default:
    return false;

  case PM_loop:
    return true;

  case PM_play:
    if (_play_rate < 0.0f) {
      // If we're playing backwards, we must be in front of the beginning.
      return _cycle > _start_cycle;
    } else {
      return _cycle < (_start_cycle + _play_cycles);
    }
  }
}

/**
 * Calculates the current cycle value for the animation playing on the layer.
 */
void AnimLayer::
accumulate_cycle() {
  if (_play_mode == PM_pose) {
    _cycle = _unclamped_cycle = _start_cycle;

  } else if (_play_mode == PM_none) {
    _cycle = _unclamped_cycle = 0.0f;

  } else {
    nassertv(_sequence >= 0);

    ClockObject *global_clock = ClockObject::get_global_clock();
    double now = global_clock->get_frame_time();

    if (_last_advance_time == 0.0f) {
      _last_advance_time = now;
    }

    double elapsed = now - _last_advance_time;

    if (elapsed <= 0.001f) {
      return;
    }

    // Note animation time for next frame.
    _last_advance_time = now;

    AnimChannel *channel = _character->get_channel(_sequence);
    nassertv(channel != nullptr);

    // Accumulate into full unclamped cycle.
    _unclamped_cycle += elapsed * (channel->get_cycle_rate(_character) * _play_rate);

    // Now clamp and wrap it based on the selected play mode.
    _cycle = clamp_cycle(_unclamped_cycle);
  }
}

/**
 * Returns a wrapped and clamped cycle value from the indicated full cycle.
 * Takes into account play mode and playback range.
 */
PN_stdfloat AnimLayer::
clamp_cycle(PN_stdfloat c) const {
  if (_play_mode == PM_play || _play_mode == PM_pose) {
    return std::min(std::max(c, 0.0f), _play_cycles) + _start_cycle;

  } else if (_play_mode == PM_loop) {
    nassertr(_play_cycles >= 0.0f, 0.0f);
    return cmod(c, _play_cycles) + _start_cycle;

  } else if (_play_mode == PM_pingpong) {
    // Pingpong.
    nassertr(_play_cycles >= 0.0f, 0.0f);
    c = cmod(c, _play_cycles * 2.0f);
    if (c > _play_cycles) {
      return (_play_cycles * 2.0f - c) + _start_cycle;
    } else {
      return c + _start_cycle;
    }

  } else {
    return c;
  }
}


/**
 *
 */
void AnimLayer::
update() {
  if (_sequence < 0) {
    _sequence_finished = true;
    return;
  }

  _prev_cycle = _cycle;
  accumulate_cycle();

  if (!is_playing()) {
    _sequence_finished = true;
  }

  _ramp_weight = 1;

  if (is_autoramp()) {
    if (_play_mode == PM_play || _play_mode == PM_pose) {
      PN_stdfloat rel_cycle = (_cycle - _start_cycle) / _play_cycles;
      if (_blend_in != 0.0f) {
        if (rel_cycle < _blend_in) {
          _ramp_weight = rel_cycle / _blend_in;
        }
      }

      if (_blend_out != 0.0f) {
        if (rel_cycle > (1.0f - _blend_out)) {
          _ramp_weight = (1.0f - rel_cycle) / _blend_out;
        }
      }

    } else if (_play_mode == PM_loop || _play_mode == PM_pingpong) {
      // Looping only blends in since it never ends.  It can't blend in based
      // on the current cycle because the cycle goes back to the beginning
      // every time it loops around, creating an incorrect effect.

      // Blend in looping animations using the unclamped cycle value.  This way
      // it only blends in during the first loop.

      if (_blend_in != 0.0f) {
        PN_stdfloat rel_cycle = (std::abs(_unclamped_cycle) - _start_cycle) / _play_cycles;
        if (rel_cycle < _blend_in) {
          _ramp_weight = rel_cycle / _blend_in;
        }
      }
    }

    _ramp_weight *= 3.0 * _ramp_weight * _ramp_weight - 2.0 * _ramp_weight * _ramp_weight * _ramp_weight;
    if (_sequence == 0) {
      _ramp_weight = 0;
    }
  }
}

/**
 *
 */
void AnimLayer::
calc_pose(AnimEvalContext &context, AnimEvalData &data, bool transition) {
  if (_sequence < 0) {
    return;
  }

  context._play_mode = _play_mode;
  context._start_cycle = _start_cycle;
  context._play_cycles = _play_cycles;
  context._play_rate = _play_rate;

  AnimChannel *channel = _character->get_channel(_sequence);
  nassertv(channel != nullptr);

  data._cycle = _cycle;
  data._weight = _weight;
  channel->calc_pose(context, data);

  if (transition) {
    // Maintain our sequence transitions.

    if (_transition_queue.empty()) {
      _transition_queue.push_back(AnimLayer());
    }

    AnimLayer *current_blend = &_transition_queue.back();
    if (current_blend->_layer_anim_time > 0.0f &&
        (current_blend->_sequence != _sequence || (_sequence_parity != _prev_sequence_parity))) {
      // Sequence changed.

      if (channel->has_flags(AnimChannel::F_snap)) {
        // Channel shouldn't be transitioned to.  Remove all entries.
        _transition_queue.clear();

      } else {
        AnimChannel *prev_channel = _character->get_channel(current_blend->_sequence);
        nassertv(prev_channel != nullptr);
        current_blend->_layer_fade_out_time = std::min(prev_channel->get_fade_out(),
                                                      channel->get_fade_in());
      }

      // Push previously set sequence.
      _transition_queue.push_back(AnimLayer());
      current_blend = &_transition_queue.back();
    }

    _prev_sequence_parity = _sequence_parity;

    ClockObject *global_clock = ClockObject::get_global_clock();

    // Keep track of current sequence.
    current_blend->_sequence = _sequence;
    current_blend->_play_mode = _play_mode;
    current_blend->_start_cycle = _start_cycle;
    current_blend->_play_cycles = _play_cycles;
    current_blend->_layer_anim_time = global_clock->get_frame_time();
    current_blend->_cycle = _cycle;
    current_blend->_play_rate = _play_rate;

    // Calculate blending weights for previous sequences.
    for (int i = 0; i < (int)_transition_queue.size() - 1;) {
      PN_stdfloat s = _transition_queue[i].get_fade_out(global_clock->get_frame_time());
      if (s > 0.0f) {
        _transition_queue[i]._weight = s;
        i++;
      } else {
        i = _transition_queue.erase(_transition_queue.begin() + i) - _transition_queue.begin();
      }
    }

    // Process previous sequences.
    for (int i = (int)_transition_queue.size() - 2; i >= 0; i--) {
      AnimLayer *blend = &_transition_queue[i];
      AnimChannel *blend_channel = _character->get_channel(blend->_sequence);
      nassertv(blend_channel != nullptr);

      // Calculate what the cycle would be if the channel kept playing.
      PN_stdfloat cycle;
      if (blend->_play_mode == PM_pose ||
          blend->_play_mode == PM_none) {
        cycle = blend->_cycle;
      } else {
        PN_stdfloat dt = (context._time - blend->_layer_anim_time);
        cycle = blend->_cycle + (dt * blend->_play_rate *
                                 blend_channel->get_cycle_rate(_character));
        cycle = blend->clamp_cycle(cycle);
      }

      context._play_mode = blend->_play_mode;
      context._start_cycle = blend->_start_cycle;
      context._play_cycles = blend->_play_cycles;
      context._play_rate = blend->_play_rate;

      data._cycle = cycle;
      data._weight = blend->_weight;
      blend_channel->calc_pose(context, data);
    }

  } else {
    _prev_sequence_parity = _sequence_parity;
  }
}
