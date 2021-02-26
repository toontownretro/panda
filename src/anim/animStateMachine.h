/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animStateMachine.h
 * @author lachbr
 * @date 2021-02-21
 */

#ifndef ANIMSTATEMACHINE_H
#define ANIMSTATEMACHINE_H

#include "animGraphNode.h"
#include "pmap.h"

/**
 * Animation graph node that contains multiple subgraphs, treated as different
 * animation states.  Subgraphs may transition between each other.
 */
class EXPCL_PANDA_ANIM AnimStateMachine final : public AnimGraphNode {
PUBLISHED:
  AnimStateMachine(const std::string &name);

  void set_state(const std::string &state);

  void add_state(const std::string &name, AnimGraphNode *graph,
                 PN_stdfloat fade_in = 0.2f, PN_stdfloat fade_out = 0.2f);

protected:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  class State {
  public:
    PT(AnimGraphNode) _graph;
    // The time to transition into this state from another state.
    PN_stdfloat _fade_in;
    // The time to transition out of this state into another state.
    PN_stdfloat _fade_out;

    PN_stdfloat _weight;

    std::string _name;
  };

  typedef pmap<std::string, State> States;
  States _states;

  State *_current_state;
  State *_last_state;
  PN_stdfloat _state_change_time;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimGraphNode::init_type();
    register_type(_type_handle, "AnimStateMachine",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animStateMachine.I"

#endif // ANIMSTATEMACHINE_H
