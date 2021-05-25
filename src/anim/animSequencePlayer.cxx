/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequencePlayer.cxx
 * @author brian
 * @date 2021-05-19
 */

#include "animSequencePlayer.h"
#include "clockObject.h"

TypeHandle AnimSequencePlayer::_type_handle;

static constexpr PN_stdfloat max_anim_time_interval = 0.2f;

/**
 *
 */
AnimSequencePlayer::
AnimSequencePlayer(const std::string &name, Character *character) :
  AnimGraphNode(name),
  _character(character),
  _prev_anim_time(ClockObject::get_global_clock()->get_frame_time()),
  _anim_time(ClockObject::get_global_clock()->get_frame_time()),
  _cycle(0.0f),
  _play_rate(1.0f),
  _sequence_finished(false),
  _sequence_loops(false),
  _sequence(-1),
  _transitions_enabled(true),
  _advance_mode(AM_auto),
  _new_sequence_parity(0),
  _prev_sequence_parity(0)
{
}

/**
 *
 */
void AnimSequencePlayer::
reset_sequence(int sequence) {
  bool changed = sequence != _sequence;
  set_sequence(sequence);

  if (changed || !_character->get_sequence(_sequence)->has_flags(AnimSequence::F_looping)) {
    reset_sequence_info();
    set_cycle(0.0f);
  }
}

/**
 *
 */
void AnimSequencePlayer::
reset_sequence_info() {
  if (_sequence == -1) {
    set_sequence(0);
  }

  _sequence_loops = _character->get_sequence(_sequence)->has_flags(AnimSequence::F_looping);
  _sequence_finished = false;
  _play_rate = 1.0f;

  _new_sequence_parity = (_new_sequence_parity + 1) % 256;
  //_last_event
}

/**
 * Advances the animation time and drives the cycles of the base sequence and
 * any active layers.
 */
void AnimSequencePlayer::
advance() {
  PN_stdfloat layer_advance = get_anim_time_interval();

  if (_prev_anim_time == 0.0f) {
    _prev_anim_time = _anim_time;
  }

  ClockObject *global_clock = ClockObject::get_global_clock();

  // Time since last animation.
  PN_stdfloat interval = global_clock->get_frame_time() - _anim_time;
  interval = std::max(0.0f, std::min(max_anim_time_interval, interval));

  if (interval <= 0.001f) {
    return;
  }

  _prev_anim_time = _anim_time;
  _anim_time = global_clock->get_frame_time();

  if (_sequence == -1) {
    return;
  }

  AnimSequence *seq = _character->get_sequence(_sequence);

  // Drive cycle.
  PN_stdfloat cycle_rate = get_sequence_cycle_rate(_sequence) * _play_rate;
  PN_stdfloat cycle_delta = interval * cycle_rate;
  PN_stdfloat new_cycle = _cycle + cycle_delta;

  if (new_cycle < 0.0f || new_cycle >= 1.0f) {
    if (_sequence_loops) {
      new_cycle -= (int)new_cycle;
    } else {
      new_cycle = (new_cycle < 0.0f) ? 0.0f : 1.0f;
    }
    _sequence_finished = true;

  } else if (new_cycle > get_last_visible_cycle(_sequence)) {
    _sequence_finished = true;
  }

  _cycle = new_cycle;

  // Advance our layers.
  for (int i = 0; i < (int)_layers.size(); i++) {
    AnimLayer *layer = &_layers[i];

    if (layer->is_active()) {
      if (layer->is_killme()) {
        if (layer->_kill_delay > 0.0f) {
          layer->_kill_delay -= layer_advance;
          layer->_kill_delay = std::max(0.0f, std::min(1.0f, layer->_kill_delay));

        } else if (layer->_weight != 0.0f) {
          // Give it at least one frame advance cycle to propagate 0.0 to client.
          layer->_weight -= layer->_kill_rate * layer_advance;
          layer->_weight = std::max(0.0f, std::min(1.0f, layer->_weight));

        } else {
          // Shift the other layers down in order
          fast_remove_layer(i);
          // Needs at least one thing cycle dead to trigger sequence change.
          layer->dying();
          continue;
        }
      }

      layer->advance(layer_advance, this);
      if (layer->_sequence_finished && layer->is_autokill()) {
        layer->_weight = 0.0f;
        layer->killme();
      }
    } else if (layer->is_dying()) {
      layer->dead();

    } else if (layer->_weight > 0.0f) {
      // Now that the server blends, it is turning off layers all the time.
      layer->init(this);
      layer->dying();
    }
  }
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_sequence_cycle_rate(int sequence) const {
  nassertr(sequence >= 0 && sequence < _character->get_num_sequences(), 0.0f);
  AnimSequence *seq = _character->get_sequence(sequence);
  PN_stdfloat t = seq->get_length();
  if (t > 0.0f) {
    return 1.0f / t;
  } else {
    return 1.0f / 0.1f;
  }
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_last_visible_cycle(int sequence) const {
  nassertr(sequence >= 0 && sequence < _character->get_num_sequences(), 0.0f);

  AnimSequence *seq = _character->get_sequence(sequence);
  if (!seq->has_flags(AnimSequence::F_looping)) {
    return 1.0f - seq->get_fade_out() * get_sequence_cycle_rate(sequence) * _play_rate;
  } else {
    return 1.0f;
  }
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_anim_time_interval() const {
  ClockObject *global_clock = ClockObject::get_global_clock();

  PN_stdfloat interval;
  if (_anim_time < global_clock->get_frame_time()) {
    // Estimate what it will be this frame.
    interval = std::max(0.0f, std::min(max_anim_time_interval, (PN_stdfloat)global_clock->get_frame_time() - _anim_time));

  } else {
    // Report actual.
    interval = std::max(0.0f, std::min(max_anim_time_interval, _anim_time - _prev_anim_time));
  }

  return interval;
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
clamp_cycle(PN_stdfloat cycle, bool is_looping) const {
  if (is_looping) {
    cycle -= (int)cycle;
    if (cycle < 0.0f) {
      cycle += 1.0f;
    }

  } else {
    cycle = std::max(0.0f, std::min(0.999f, cycle));
  }

  return cycle;
}

/**
 *
 */
int AnimSequencePlayer::
add_gesture_sequence(int sequence, bool auto_kill) {
  int i = add_layered_sequence(sequence, 0);
  // No room?
  if (is_valid_layer(i)) {
    set_layer_auto_kill(i, auto_kill);
  }
  return i;
}

/**
 *
 */
int AnimSequencePlayer::
add_gesture_sequence(int sequence, PN_stdfloat duration, bool auto_kill) {
  int i = add_layered_sequence(sequence, auto_kill);
  if (i > 0 && duration > 0) {
    _layers[i]._play_rate = _character->get_sequence(sequence)->get_length() / duration;
  }
  return i;
}

/**
 *
 */
int AnimSequencePlayer::
add_gesture(int activity, int sequence, bool auto_kill) {
  if (is_playing_gesture(activity)) {
    return find_gesture_layer(activity);
  }

  int i = add_gesture_sequence(sequence, auto_kill);
  if (i != -1) {
    _layers[i]._activity = activity;
  }

  return i;
}

/**
 *
 */
int AnimSequencePlayer::
add_gesture_with_duration(int activity, int sequence, PN_stdfloat duration, bool auto_kill) {
  int i = add_gesture(activity, sequence, auto_kill);
  set_layer_duration(i, duration);

  return i;
}

/**
 *
 */
bool AnimSequencePlayer::
is_playing_gesture(int activity) const {
  return find_gesture_layer(activity) != -1;
}

/**
 * Resets an existing layer to the specified activity.
 */
void AnimSequencePlayer::
reset_layer(int layer, int activity, int sequence, bool auto_kill) {
  nassertv(layer >= 0 && layer < (int)_layers.size());

  _layers[layer]._activity = activity;
  _layers[layer]._order = layer;
  _layers[layer]._priority = 0;
  _layers[layer]._cycle = 0.0f;
  _layers[layer]._prev_cycle = 0.0f;
  _layers[layer]._play_rate = 1.0f;
  _layers[layer]._sequence = sequence;
  _layers[layer]._weight = 1.0f;
  _layers[layer]._blend_in = 0.0f;
  _layers[layer]._blend_out = 0.0f;
  _layers[layer]._sequence_finished = false;
  _layers[layer]._last_event_check = ClockObject::get_global_clock()->get_frame_time();
  _layers[layer]._looping = _character->get_sequence(sequence)->has_flags(AnimSequence::F_looping);
  if (auto_kill) {
    _layers[layer]._flags |= AnimLayer::F_autokill;
  } else {
    _layers[layer]._flags &= ~AnimLayer::F_autokill;
  }
  _layers[layer]._flags |= AnimLayer::F_active;
  _layers[layer]._sequence_parity = (_layers[layer]._sequence_parity + 1) % 256;
  _layers[layer].mark_active();
}

/**
 *
 */
void AnimSequencePlayer::
restart_gesture(int activity, bool add_if_missing, bool auto_kill) {
  int idx = find_gesture_layer(activity);
  if (idx == -1) {
    if (add_if_missing) {
      add_gesture(activity, auto_kill);
    }
    return;
  }

  _layers[idx]._cycle = 0.0;
  _layers[idx]._prev_cycle = 0.0;
  _layers[idx]._last_event_check = 0.0;
  _layers[idx]._sequence_parity = (_layers[idx]._sequence_parity + 1) % 256;
  _layers[idx].mark_active();
}

/**
 *
 */
void AnimSequencePlayer::
remove_gesture(int activity) {
  int layer = find_gesture_layer(activity);
  if (layer == -1) {
    return;
  }

  remove_layer(layer);
}

/**
 *
 */
void AnimSequencePlayer::
remove_all_gestures() {
  for (int i = 0; i < (int)_layers.size(); i++) {
    remove_layer(i);
  }
}

/**
 *
 */
int AnimSequencePlayer::
add_layered_sequence(int sequence, int priority) {
  int i = allocate_layer(priority);
  if (is_valid_layer(i)) {
    AnimLayer *layer = &_layers[i];
    layer->_cycle = 0;
    layer->_prev_cycle = 0;
    layer->_play_rate = 1.0;
    layer->_activity = -1;
    layer->_sequence = sequence;
    layer->_weight = 1.0;
    layer->_blend_in = 0.0;
    layer->_blend_out = 0.0;
    layer->_sequence_finished = false;
    layer->_last_event_check = 0;
    layer->_looping = _character->get_sequence(sequence)->has_flags(AnimSequence::F_looping);
    layer->_sequence_parity = (layer->_sequence_parity + 1) % 256;
  }
  return i;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_priority(int layer, int priority) {
  if (!is_valid_layer(layer)) {
    return;
  }

  if (_layers[layer]._priority == priority) {
    return;
  }

  // Look for an open slot and for existing layers that are lower priority.
  int i;
  for (i = 0; i < (int)_layers.size(); i++) {
    if (_layers[i].is_active()) {
      if (_layers[i]._order > _layers[layer]._order) {
        _layers[i]._order--;
      }
    }
  }

  int new_order = 0;
  for (i = 0; i < (int)_layers.size(); i++) {
    if (i != layer && _layers[i].is_active()) {
      if (_layers[i]._priority <= priority) {
        new_order = std::max(new_order, _layers[i]._order + 1);
      }
    }
  }

  for (i = 0; i < (int)_layers.size(); i++) {
    if (i != layer && _layers[i].is_active()) {
      if (_layers[i]._order >= new_order) {
        _layers[i]._order++;
      }
    }
  }

  _layers[layer]._order = new_order;
  _layers[layer]._priority = priority;
  _layers[layer].mark_active();
}

/**
 *
 */
bool AnimSequencePlayer::
is_valid_layer(int layer) const {
  return (layer >= 0 && layer < (int)_layers.size() && _layers[layer].is_active());
}

/**
 *
 */
int AnimSequencePlayer::
allocate_layer(int priority) {
  int new_order = 0;
  int open_layer = -1;
  int num_open = 0;
  int i;

  // Look for an open slot and for existing layers that are lower priority.
  for (i = 0; i < (int)_layers.size(); i++) {
    if (_layers[i].is_active()) {
      if (_layers[i]._priority <= priority) {
        new_order = std::max(new_order, _layers[i]._order + 1);
      }
    } else if (_layers[i].is_dying()) {
      // Skip.
    } else if (open_layer == -1) {
      open_layer = i;
    } else {
      num_open++;
    }
  }

  if (open_layer == -1) {
    if (_layers.size() >= max_anim_layers) {
      return -1;
    }

    open_layer = (int)_layers.size();
    _layers.push_back(AnimLayer());
    _layers[open_layer].init(this);
  }

  // Make sure there's always an empty unused layer
  if (num_open == 0) {
    if (_layers.size() < max_anim_layers) {
      i = (int)_layers.size();
      _layers.push_back(AnimLayer());
      _layers[i].init(this);
    }
  }

  for (i = 0; i < (int)_layers.size(); i++) {
    if (_layers[i]._order >= new_order && _layers[i]._order < max_anim_layers) {
      _layers[i]._order++;
    }
  }

  _layers[open_layer]._flags = AnimLayer::F_active;
  _layers[open_layer]._order = new_order;
  _layers[open_layer]._priority = priority;
  _layers[open_layer].mark_active();

  return open_layer;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_duration(int layer, PN_stdfloat duration) {
  if (is_valid_layer(layer) && duration > 0) {
    _layers[layer]._play_rate = _character->get_sequence(_layers[layer]._sequence)->get_length() / duration;
  }
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_duration(int layer) const {
  if (is_valid_layer(layer)) {
    if (_layers[layer]._play_rate != 0.0) {
      return (1.0f - _layers[layer]._cycle) * _character->get_sequence(_layers[layer]._sequence)->get_length() / _layers[layer]._play_rate;
    }
    return _character->get_sequence(_layers[layer]._sequence)->get_length();
  }

  return 0.0f;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_cycle(int layer, PN_stdfloat cycle) {
  if (!is_valid_layer(layer)) {
    return;
  }

  if (!_layers[layer]._looping) {
    cycle = std::max(0.0f, std::min(1.0f, cycle));
  }

  _layers[layer]._cycle = cycle;
  _layers[layer].mark_active();
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_cycle(int layer, PN_stdfloat cycle, PN_stdfloat prev_cycle) {
  if (!is_valid_layer(layer)) {
    return;
  }

  if (!_layers[layer]._looping) {
    cycle = std::max(0.0f, std::min(1.0f, cycle));
    prev_cycle = std::max(0.0f, std::min(1.0f, prev_cycle));
  }

  _layers[layer]._cycle = cycle;
  _layers[layer]._prev_cycle = prev_cycle;
  _layers[layer]._last_event_check = prev_cycle;
  _layers[layer].mark_active();
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_cycle(int layer) const {
  if (!is_valid_layer(layer)) {
    return 0.5f;
  }

  return _layers[layer]._cycle;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_prev_cycle(int layer, PN_stdfloat cycle) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._prev_cycle = cycle;
  _layers[layer]._last_event_check = cycle;
  _layers[layer].mark_active();
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_prev_cycle(int layer) const {
  if (!is_valid_layer(layer)) {
    return 0.0f;
  }

  return _layers[layer]._prev_cycle;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_play_rate(int layer, PN_stdfloat play_rate) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._play_rate = play_rate;
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_play_rate(int layer) const {
  if (!is_valid_layer(layer)) {
    return 0.0f;
  }

  return _layers[layer]._play_rate;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_weight(int layer, PN_stdfloat weight) {
  if (!is_valid_layer(layer)) {
    return;
  }

  weight = std::max(0.0f, std::min(1.0f, weight));
  _layers[layer]._weight = weight;
  _layers[layer].mark_active();
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_weight(int layer) const {
  if (!is_valid_layer(layer)) {
    return 0.0f;
  }

  return _layers[layer]._weight;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_blend_in(int layer, PN_stdfloat blend_in) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._blend_in = blend_in;
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_blend_in(int layer) const {
  if (!is_valid_layer(layer)) {
    return 0.0f;
  }

  return _layers[layer]._blend_in;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_blend_out(int layer, PN_stdfloat blend_out) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._blend_out = blend_out;
}

/**
 *
 */
PN_stdfloat AnimSequencePlayer::
get_layer_blend_out(int layer) const {
  if (!is_valid_layer(layer)) {
    return 0.0f;
  }

  return _layers[layer]._blend_out;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_order(int layer, int order) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._order = order;
}

/**
 *
 */
int AnimSequencePlayer::
get_layer_order(int layer) const {
  if (!is_valid_layer(layer)) {
    return max_anim_layers;
  }

  return _layers[layer]._order;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_auto_kill(int layer, bool auto_kill) {
  if (!is_valid_layer(layer)) {
    return;
  }

  if (auto_kill) {
    _layers[layer]._flags |= AnimLayer::F_autokill;
  } else {
    _layers[layer]._flags &= ~AnimLayer::F_autokill;
  }
}

/**
 *
 */
bool AnimSequencePlayer::
get_layer_auto_kill(int layer) const {
  if (!is_valid_layer(layer)) {
    return false;
  }

  return (_layers[layer]._flags & AnimLayer::F_autokill) != 0;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_looping(int layer, bool looping) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._looping = looping;
}

/**
 *
 */
bool AnimSequencePlayer::
get_layer_looping(int layer) const {
  if (!is_valid_layer(layer)) {
    return false;
  }

  return _layers[layer]._looping;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_no_restore(int layer, bool no_restore) {
  if (!is_valid_layer(layer)) {
    return;
  }

  if (no_restore) {
    _layers[layer]._flags |= AnimLayer::F_dontrestore;
  } else {
    _layers[layer]._flags &= ~AnimLayer::F_dontrestore;
  }
}

/**
 *
 */
bool AnimSequencePlayer::
get_layer_no_resture(int layer) const {
  if (!is_valid_layer(layer)) {
    return false;
  }

  return (_layers[layer]._flags & AnimLayer::F_dontrestore) != 0;
}

/**
 *
 */
void AnimSequencePlayer::
mark_layer_active(int layer) {
  if (layer < 0 || layer >= (int)_layers.size()) {
    return;
  }

  _layers[layer]._flags = AnimLayer::F_active;
  _layers[layer].mark_active();
}

/**
 *
 */
int AnimSequencePlayer::
get_layer_activity(int layer) const {
  if (!is_valid_layer(layer)) {
    return -1;
  }

  return _layers[layer]._activity;
}

/**
 *
 */
void AnimSequencePlayer::
set_layer_sequence(int layer, int seq) {
  if (!is_valid_layer(layer)) {
    return;
  }

  _layers[layer]._sequence = seq;
}

/**
 *
 */
int AnimSequencePlayer::
get_layer_sequence(int layer) const {
  if (!is_valid_layer(layer)) {
    return -1;
  }

  return _layers[layer]._sequence;
}

/**
 *
 */
int AnimSequencePlayer::
find_gesture_layer(int activity) const {
  for (int i = 0; i < (int)_layers.size(); i++) {
    if (!_layers[i].is_active()) {
      continue;
    }
    if (_layers[i].is_killme()) {
      continue;
    }
    if (_layers[i]._activity == -1) {
      continue;
    }
    if (_layers[i]._activity == activity) {
      return i;
    }
  }

  return -1;
}

/**
 *
 */
void AnimSequencePlayer::
remove_layer(int layer, PN_stdfloat kill_rate, PN_stdfloat kill_delay) {
  if (!is_valid_layer(layer)) {
    return;
  }

  if (kill_rate > 0.0f) {
    _layers[layer]._kill_rate = _layers[layer]._weight / kill_rate;
  } else {
    _layers[layer]._kill_rate = 100;
  }

  _layers[layer]._kill_delay = kill_delay;
  _layers[layer].killme();
}

/**
 *
 */
void AnimSequencePlayer::
fast_remove_layer(int layer) {
  if (!is_valid_layer(layer)) {
    return;
  }

  // Shift the other layers down in order.
  for (int j = 0; j < (int)_layers.size(); j++) {
    if (_layers[j].is_active() && _layers[j]._order > _layers[layer]._order) {
      _layers[j]._order--;
    }
  }

  _layers[layer].init(this);
}

/**
 *
 */
AnimLayer *AnimSequencePlayer::
get_layer(int i) {
  i = std::max(0, std::min((int)_layers.size(), i));
  return &_layers[i];
}

/**
 *
 */
void AnimSequencePlayer::
set_num_layers(int count) {
  _layers.resize(count);
}

/**
 *
 */
int AnimSequencePlayer::
get_num_layers() const {
  return (int)_layers.size();
}

/**
 *
 */
bool AnimSequencePlayer::
has_active_layer() const {
  for (int i = 0; i < (int)_layers.size(); i++) {
    if (_layers[i].is_active()) {
      return true;
    }
  }

  return false;
}

/**
 *
 */
void AnimSequencePlayer::
evaluate(AnimGraphEvalContext &context) {
  if (_sequence == -1) {
    return;
  }

  nassertv(context._character == _character);

  // If the advance mode is AM_auto, advance the cycle now.
  if (_advance_mode == AM_auto) {
    advance();
  }

  AnimSequence *seq = _character->get_sequence(_sequence);
  context._cycle = _cycle;
  context._weight = 1.0f;
  seq->evaluate(context);

  if (_transitions_enabled) {
    // Maintain our sequence transitions.

    if (_sequence_queue.empty()) {
      _sequence_queue.push_back(AnimLayer());
    }

    AnimLayer *current_blend = &_sequence_queue[_sequence_queue.size() - 1];
    if (current_blend->_layer_anim_time &&
        (current_blend->_sequence != _sequence || (_new_sequence_parity != _prev_sequence_parity))) {

      // Sequence changed.
      if (seq->has_flags(AnimSequence::F_snap)) {
        // Remove all entries.
        _sequence_queue.clear();

      } else {
        AnimSequence *prev_seq = _character->get_sequence(current_blend->_sequence);
        current_blend->_layer_fade_out_time = std::min(prev_seq->get_fade_out(), seq->get_fade_in());
      }

      // Push previously set sequence.
      _sequence_queue.push_back(AnimLayer());
      current_blend = &_sequence_queue[_sequence_queue.size() - 1];
    }

    _prev_sequence_parity = _new_sequence_parity;

    ClockObject *global_clock = ClockObject::get_global_clock();

    // Keep track of current sequence.
    current_blend->_sequence = _sequence;
    current_blend->_layer_anim_time = global_clock->get_frame_time();
    current_blend->_cycle = _cycle;
    current_blend->_play_rate = _play_rate;

    // Calculate blending weights for previous sequences.
    for (int i = 0; i < (int)_sequence_queue.size() - 1;) {
      PN_stdfloat s = _sequence_queue[i].get_fade_out(global_clock->get_frame_time());
      if (s > 0.0f) {
        _sequence_queue[i]._weight = s;
        i++;
      } else {
        i = _sequence_queue.erase(_sequence_queue.begin() + i) - _sequence_queue.begin();
      }
    }

    // Process previous sequences.
    for (int i = (int)_sequence_queue.size() - 2; i >= 0; i--) {
      AnimLayer *blend = &_sequence_queue[i];
      AnimSequence *blend_seq = _character->get_sequence(blend->_sequence);
      PN_stdfloat dt = (global_clock->get_frame_time() - blend->_layer_anim_time);
      PN_stdfloat cycle = blend->_cycle + dt * blend->_play_rate * get_sequence_cycle_rate(blend->_sequence);
      cycle = clamp_cycle(cycle, blend_seq->has_flags(AnimSequence::F_looping));
      context._cycle = cycle;
      context._weight = blend->_weight;
      blend_seq->evaluate(context);
    }
  } else {
    _prev_sequence_parity = _new_sequence_parity;
  }

  // Sort the layers.
  int layer[max_anim_layers];
  for (int i = 0; i < (int)_layers.size(); i++) {
    layer[i] = max_anim_layers;
  }

  for (int i = 0; i < (int)_layers.size(); i++) {
    AnimLayer *thelayer = &_layers[i];
    if ((thelayer->_weight > 0.0f) && (thelayer->is_active()) && thelayer->_order >= 0 && thelayer->_order < (int)_layers.size()) {
      layer[thelayer->_order] = i;
    }
  }

  for (int i = 0; i < (int)_layers.size(); i++) {
    AnimLayer *thelayer = &_layers[i];
    if ((layer[i] >= 0) && (layer[i] < (int)_layers.size()) &&
        (thelayer->_sequence >= 0) && (thelayer->_sequence < (int)_character->get_num_sequences())) {

      AnimSequence *seq = _character->get_sequence(thelayer->_sequence);
      context._weight = std::min(1.0f, thelayer->_weight);
      context._cycle = clamp_cycle(thelayer->_cycle, seq->has_flags(AnimSequence::F_looping));
      seq->evaluate(context);
    }
  }
}
