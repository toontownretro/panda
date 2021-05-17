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
#include "animSequence.h"
#include "pmap.h"

/**
 * Animation graph node that contains multiple subgraphs, treated as different
 * animation states.  Subgraphs may transition between each other.
 */
class EXPCL_PANDA_ANIM AnimStateMachine final : public AnimGraphNode {
PUBLISHED:
  AnimStateMachine(const std::string &name);

  bool set_state(const std::string &name);
  bool set_state(int n);

  int get_state(const std::string &name) const;

  int add_state(const std::string &name, AnimSequence *seq);

protected:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  class State {
  public:
    PT(AnimSequence) _graph;
    PN_stdfloat _weight;
    std::string _name;
  };

  typedef pvector<State> States;
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
