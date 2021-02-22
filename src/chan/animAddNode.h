/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animAddNode.h
 * @author lachbr
 * @date 2021-02-18
 */

#ifndef ANIMADDNODE_H
#define ANIMADDNODE_H

#include "animGraphNode.h"

/**
 * Animation graph node that adds an input additive pose onto an input base
 * pose.  The weight of the addition can be controlled with an input alpha
 * value.  The first input is the base pose, and the second input is the pose
 * to add onto the base pose.
 */
class EXPCL_PANDA_CHAN AnimAddNode final : public AnimGraphNode {
PUBLISHED:
  AnimAddNode(const std::string &name);

  INLINE void set_base(AnimGraphNode *base);
  INLINE AnimGraphNode *get_base() const;

  INLINE void set_add(AnimGraphNode *add);
  INLINE AnimGraphNode *get_add() const;

  INLINE void set_alpha(PN_stdfloat alpha);
  INLINE PN_stdfloat get_alpha() const;

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  PT(AnimGraphNode) _base;
  PT(AnimGraphNode) _add;
  PN_stdfloat _alpha;

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
    register_type(_type_handle, "AnimAddNode",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animAddNode.I"

#endif // ANIMADDNODE_H
