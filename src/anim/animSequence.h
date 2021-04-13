/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequence.h
 * @author lachbr
 * @date 2021-03-01
 */

#ifndef ANIMSEQUENCE_H
#define ANIMSEQUENCE_H

#include "pandabase.h"
#include "animGraphNode.h"
#include "animControl.h"
#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataWriter.h"
#include "cycleDataReader.h"

/**
 *
 */
class EXPCL_PANDA_ANIM AnimSequence final : public AnimGraphNode {
PUBLISHED:
  INLINE AnimSequence(const std::string &name, AnimGraphNode *base = nullptr);

  // Replication of AnimControl interfaces that simply call into the
  // AnimControl.
  void play();
  void play(double from, double to);
  void loop(bool restart);
  void loop(bool restart, double from, double to);
  void pingpong(bool restart);
  void pingpong(bool restart, double from, double to);
  void stop();
  void pose(double frame);
  void set_play_rate(double play_rate);

  INLINE AnimControl *get_effective_control() const;

  INLINE void set_base(AnimGraphNode *base);
  INLINE void add_layer(AnimGraphNode *layer);

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  void compute_effective_control();
  void r_compute_effective_control(AnimGraphNode *node);

private:
  // The effective control is the AnimControl below us with the highest frame
  // count.  This control will be used to determine frame-based logic such as
  // events and IK rules.
  AnimControl *_effective_control;

  // All of the AnimControl nodes below this node.  Used to propagate all
  // play/loop/stop calls on the sequence to all the AnimControls below.
  typedef pvector<AnimControl *> AnimControls;
  AnimControls _controls;

  // Node to get the base pose from.
  PT(AnimGraphNode) _base;

  // Additive layers on top of base pose.
  typedef pvector<PT(AnimGraphNode)> AnimNodes;
  AnimNodes _add_layers;

  // IK Locks

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
    register_type(_type_handle, "AnimSequence",
                  AnimGraphNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animSequence.I"

#endif // ANIMSEQUENCE_H
