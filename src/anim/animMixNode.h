/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animMixNode.h
 * @author lachbr
 * @date 2021-02-18
 */

#ifndef ANIMMIXNODE_H
#define ANIMMIXNODE_H

#include "animGraphNode.h"

/**
 * Animation graph node that blends between two input nodes based on an input
 * alpha value.
 */
class EXPCL_PANDA_ANIM AnimMixNode final : public AnimGraphNode {
PUBLISHED:
  AnimMixNode(const std::string &name);

  INLINE void set_a(AnimGraphNode *a);
  INLINE AnimGraphNode *get_a() const;

  INLINE void set_b(AnimGraphNode *b);
  INLINE AnimGraphNode *get_b() const;

  INLINE void set_alpha(PN_stdfloat alpha);
  INLINE PN_stdfloat get_alpha() const;

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  PT(AnimGraphNode) _a;
  PT(AnimGraphNode) _b;
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
    register_type(_type_handle, "AnimMixNode",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animMixNode.I"

#endif // ANIMMIXNODE_H
