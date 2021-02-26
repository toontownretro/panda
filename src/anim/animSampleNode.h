/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSampleNode.h
 * @author lachbr
 * @date 2021-02-18
 */

#ifndef ANIMSAMPLENODE_H
#define ANIMSAMPLENODE_H

#include "animGraphNode.h"
#include "animControl.h"

/**
 * Animation graph node that samples a single AnimChannel value.  Performs
 * frame blending if requested.  This is a leaf node in the graph.
 */
class EXPCL_PANDA_ANIM AnimSampleNode final : public AnimGraphNode {
PUBLISHED:
  AnimSampleNode(const std::string &name);

  INLINE void set_control(AnimControl *control);
  INLINE AnimControl *get_control() const;

  // Replication of AnimControl interfaces that simply call into the
  // AnimControl.
  virtual void play();
  virtual void play(double from, double to);
  virtual void loop(bool restart);
  virtual void loop(bool restart, double from, double to);
  virtual void pingpong(bool restart);
  virtual void pingpong(bool restart, double from, double to);
  virtual void stop();
  virtual void pose(double frame);
  virtual void set_play_rate(double play_rate);

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  PT(AnimControl) _control;

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
    register_type(_type_handle, "AnimSampleNode",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animSampleNode.I"

#endif // ANIMSAMPLENODE_H
