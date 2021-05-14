/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animOverlayNode.h
 * @author brian
 * @date 2021-05-08
 */

#ifndef ANIMOVERLAYNODE_H
#define ANIMOVERLAYNODE_H

#include "animGraphNode.h"

/**
 *
 */
class EXPCL_PANDA_ANIM AnimOverlayNode final : public AnimGraphNode {
PUBLISHED:
  INLINE AnimOverlayNode(const std::string &name, AnimGraphNode *a, AnimGraphNode *b);

  INLINE void set_a(AnimGraphNode *a);
  INLINE void set_b(AnimGraphNode *b);

  INLINE AnimGraphNode *get_a() const;
  INLINE AnimGraphNode *get_b() const;

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  AnimGraphNode *_a;
  AnimGraphNode *_b;

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
    register_type(_type_handle, "AnimOverlayNode",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animOverlayNode.I"

#endif // ANIMOVERLAYNODE_H
