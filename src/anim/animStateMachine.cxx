/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animStateMachine.cxx
 * @author lachbr
 * @date 2021-02-21
 */

#include "animStateMachine.h"
#include "clockObject.h"

TypeHandle AnimStateMachine::_type_handle;

/**
 *
 */
AnimStateMachine::
AnimStateMachine(const std::string &name):
  AnimGraphNode(name),
  _current_state(nullptr),
  _last_state(nullptr) {
}

/**
 *
 */
bool AnimStateMachine::
set_state(const std::string &name) {
  int state = get_state(name);
  if (state == -1) {
    return false;
  }

  return set_state(state);
}

/**
 * Sets the active animation state.
 */
bool AnimStateMachine::
set_state(int n) {
  State *state = &_states[n];

  // This can happen if we change state while in the process of fading out
  // a different state.
  if (_last_state != nullptr) {
    _last_state->_graph->stop();
  }

  _last_state = _current_state;
  _current_state = state;

  AnimSequence *seq = state->_graph;

  if (seq->has_flags(AnimSequence::F_looping)) {
    // Assume that a looping sequence should not restart if it's already the
    // active one.
    if (_last_state != _current_state) {
      state->_graph->loop(true);
    }
  } else {
    // A non-looping sequence always restarts even if it's the active one.
    state->_graph->play();
  }

  ClockObject *clock = ClockObject::get_global_clock();
  _state_change_time = clock->get_frame_time();
  if (seq->has_flags(AnimSequence::F_snap)) {
    _state_change_time -= seq->get_fade_out();
  }

  return true;
}

/**
 *
 */
int AnimStateMachine::
get_state(const std::string &name) const {
  for (int i = 0; i < (int)_states.size(); i++) {
    if (_states[i]._name == name) {
      return i;
    }
  }

  return -1;
}

/**
 * Adds a new state.
 */
int AnimStateMachine::
add_state(const std::string &name, AnimSequence *graph) {
  State state;
  state._graph = graph;
  state._name = name;
  state._weight = 1.0f;

  add_child(graph);

  int index = (int)_states.size();
  _states.push_back(std::move(state));

  return index;
}

/**
 *
 */
void AnimStateMachine::
evaluate(AnimGraphEvalContext &context) {
  if (_current_state == nullptr && _last_state == nullptr) {
    return;
  }

  if (_current_state == nullptr) {
    return;
  }

  ClockObject *clock = ClockObject::get_global_clock();
  PN_stdfloat transition_elapsed = clock->get_frame_time() - _state_change_time;

  AnimSequence *seq = _current_state->_graph;

  if (transition_elapsed < seq->get_fade_out() && (_current_state != _last_state)) {
    PN_stdfloat frac = std::min(1.0f, transition_elapsed / seq->get_fade_out());
    if (frac > 0 && frac <= 1.0) {
      // Do a nice spline curve.
      frac = 3 * frac * frac - 2 * frac * frac * frac;
    }
    _current_state->_weight = frac;
  } else {
    _current_state->_weight = 1.0f;
  }

  if (!_last_state || _current_state->_weight == 1.0f) {
    if (_last_state != nullptr) {
      _last_state->_graph->stop();
      _last_state = nullptr;
    }

    _current_state->_graph->evaluate(context);

  } else if (_last_state) {
    AnimGraphEvalContext cur_ctx(context);
    _current_state->_graph->evaluate(cur_ctx);

    AnimGraphEvalContext last_ctx(context);
    _last_state->_graph->evaluate(last_ctx);

    context.mix(last_ctx, cur_ctx, _current_state->_weight);
  }
}
