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
#include "weightList.h"

/**
 *
 */
class EXPCL_PANDA_ANIM AnimSequence final : public AnimGraphNode {
PUBLISHED:
  enum Flags {
    F_none = 0,

    F_delta = 1 << 0,
    // Overlay delta.
    F_post = 1 << 1,
    F_all_zeros = 1 << 2,

    // Override X value of root joint with zero.
    F_zero_root_x = 1 << 3,
    // Override Y value of root joint with zero.
    F_zero_root_y = 1 << 4,
    // Override Z value of root joint with zero.
    F_zero_root_z = 1 << 5,

    F_looping = 1 << 6,

    F_snap = 1 << 7,
  };

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
  double get_play_rate() const;
  double get_frame_rate() const;
  int get_num_frames() const;

  int get_frame() const;
  int get_next_frame() const;
  double get_frac() const;
  int get_full_frame() const;
  double get_full_fframe() const;
  bool is_playing() const;

  INLINE PN_stdfloat get_length() const;

  INLINE void set_fade_in(PN_stdfloat time);
  INLINE PN_stdfloat get_fade_in() const;

  INLINE void set_fade_out(PN_stdfloat time);
  INLINE PN_stdfloat get_fade_out() const;

  INLINE void set_flags(unsigned int flags);
  INLINE bool has_flags(unsigned int flags) const;
  INLINE unsigned int get_flags() const;
  INLINE void clear_flags(unsigned int flags);

  INLINE void set_weight_list(WeightList *list);
  INLINE WeightList *get_weight_list() const;

  INLINE AnimControl *get_effective_control() const;

  INLINE void set_base(AnimGraphNode *base);
  INLINE void add_layer(AnimGraphNode *layer, int start_frame = -1, int peak_frame = -1,
                        int tail_frame = -1, int end_frame = -1, bool spline = false,
                        bool no_blend = true);

public:
  virtual void evaluate(AnimGraphEvalContext &context) override;
  void init_pose(AnimGraphEvalContext &context);
  void blend(AnimGraphEvalContext &a, AnimGraphEvalContext &b, PN_stdfloat weight);

private:
  void compute_effective_control();
  void r_compute_effective_control(AnimGraphNode *node);

private:
  class Layer {
  public:
    PT(AnimGraphNode) _seq;
    PN_stdfloat _start_frame;
    PN_stdfloat _peak_frame;
    PN_stdfloat _tail_frame;
    PN_stdfloat _end_frame;
    bool _spline;
    bool _no_blend;
  };
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
  typedef pvector<Layer> Layers;
  Layers _layers;

  // Controls per-joint weighting of the evaluated pose.
  PT(WeightList) _weights;

  unsigned int _flags;

  PN_stdfloat _fade_in;
  PN_stdfloat _fade_out;

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
