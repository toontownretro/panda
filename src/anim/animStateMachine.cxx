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
 * Sets the active animation state.
 */
void AnimStateMachine::
set_state(const std::string &name) {
  States::iterator si = _states.find(name);
  if (si == _states.end()) {
    return;
  }

  State *state = &(*si).second;

  if (state == _current_state) {
    // Already the active state, do nothing.
    return;
  }

  _last_state = _current_state;
  _current_state = state;

  ClockObject *clock = ClockObject::get_global_clock();
  _state_change_time = clock->get_frame_time();
}

/**
 * Adds a new state.
 */
void AnimStateMachine::
add_state(const std::string &name, AnimGraphNode *graph, PN_stdfloat fade_in,
          PN_stdfloat fade_out) {
  State state;
  state._graph = graph;
  state._name = name;
  state._fade_in = fade_in;
  state._fade_out = fade_out;
  state._weight = 1.0f;

  _states[name] = std::move(state);
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

  if (transition_elapsed < _current_state->_fade_in) {
    PN_stdfloat frac = std::min(1.0f, transition_elapsed / _current_state->_fade_in);
    _current_state->_weight = frac;
  } else {
    _current_state->_weight = 1.0f;
  }

  if (!_last_state || _current_state->_weight == 1.0f) {
    _current_state->_graph->evaluate(context);

  } else if (_last_state) {
    AnimGraphEvalContext cur_ctx(context);
    _current_state->_graph->evaluate(cur_ctx);

    AnimGraphEvalContext last_ctx(context);
    _last_state->_graph->evaluate(last_ctx);

    context.mix(last_ctx, cur_ctx, _current_state->_weight);
  }
}
