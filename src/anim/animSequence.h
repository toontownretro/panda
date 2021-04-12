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
#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataWriter.h"
#include "cycleDataReader.h"

/**
 *
 */
class AnimSequence final : public AnimGraphNode {
PUBLISHED:
  enum PlayMode {
    PM_pose,
    PM_play,
    PM_loop,
    PM_pingpong,
  };

  INLINE AnimSequence(const std::string &name);

  void play();
  void play(double from, double to);
  void loop(bool restart);
  void loop(bool restart, double from, double to);
  void pingpong(bool restart);
  void pingpong(bool restart, double from, double to);
  void stop();
  void pose(double frame);

  INLINE void set_base(AnimGraphNode *base);
  INLINE void add_layer(AnimGraphNode *layer);

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;

private:
  void compute_effective_num_frames();
  void r_compute_effective_num_frames(AnimGraphNode *node);

private:

  // Animation playing/timing variables.
  PN_stdfloat _play_rate;
  double _start_time;
  double _start_frame;
  double _end_frame;
  bool _restart;
  PlayMode _play_mode;

  // This is the *effective* number of frames in the sequence.  That is, the
  // maximum number of frames of all the AnimBundles underneath this sequence.
  int _effective_num_frames;

  // Node to get the base pose from.
  PT(AnimGraphNode) _base;

  // Additive layers on top of base pose.
  typedef pvector<PT(AnimGraphNode)> AnimNodes;
  AnimNodes _add_layers;

  // IK Locks

};

#include "animSequence.I"

#endif // ANIMSEQUENCE_H
