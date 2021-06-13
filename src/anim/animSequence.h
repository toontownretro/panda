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

    // Explicit number of frames (if no base pose).
    F_num_frames = 1 << 8,

    // Explicit frame rate.
    F_frame_rate = 1 << 9,

    F_real_time = 1 << 10,
  };

  class EXPCL_PANDA_ANIM AnimEvent {
  PUBLISHED:
    INLINE AnimEvent(int type, int event, PN_stdfloat cycle, const std::string &options) :
      _type(type),
      _cycle(cycle),
      _event(event),
      _options(options)
    {}

    INLINE PN_stdfloat get_cycle() const { return _cycle; }
    INLINE int get_type() const { return _type; }
    INLINE int get_event() const { return _event; }
    INLINE const std::string &get_options() const { return _options; }

  private:
    int _type;
    PN_stdfloat _cycle;
    int _event;
    std::string _options;
  };

  INLINE AnimSequence(const std::string &name, AnimGraphNode *base = nullptr);

  void set_frame_rate(int frame_rate);
  void clear_frame_rate();
  double get_frame_rate() const;

  void set_num_frames(int num_frames);
  void clear_num_frames();
  int get_num_frames() const;

  PN_stdfloat get_length();
  PN_stdfloat get_cycles_per_second();

  void add_event(int type, int event, int frame, const std::string &options = "");
  INLINE int get_num_events() const;
  INLINE const AnimEvent &get_event(int n) const;

  INLINE void set_fade_in(PN_stdfloat time);
  INLINE PN_stdfloat get_fade_in() const;

  INLINE void set_fade_out(PN_stdfloat time);
  INLINE PN_stdfloat get_fade_out() const;

  INLINE void set_flags(unsigned int flags);
  INLINE bool has_flags(unsigned int flags) const;
  INLINE unsigned int get_flags() const;
  INLINE void clear_flags(unsigned int flags);

  INLINE void set_activity(int activity, PN_stdfloat weight = 1.0f);
  INLINE int get_activity() const;
  INLINE PN_stdfloat get_activity_weight() const;

  INLINE void set_weight_list(WeightList *list);
  INLINE WeightList *get_weight_list() const;

  INLINE void set_base(AnimGraphNode *base);
  INLINE void add_layer(AnimGraphNode *layer, int start_frame = -1, int peak_frame = -1,
                        int tail_frame = -1, int end_frame = -1, bool spline = false,
                        bool no_blend = false);

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
    PN_stdfloat _start;
    PN_stdfloat _peak;
    PN_stdfloat _tail;
    PN_stdfloat _end;
    bool _spline;
    bool _no_blend;
  };

  // All of the animations below the sequence.
  typedef pvector<AnimBundle *> Anims;
  Anims _anims;

  // Node to get the base pose from.
  PT(AnimGraphNode) _base;

  // Additive layers on top of base pose.
  typedef pvector<Layer> Layers;
  Layers _layers;

  // Controls per-joint weighting of the evaluated pose.
  PT(WeightList) _weights;

  unsigned int _flags;

  int _activity;
  PN_stdfloat _activity_weight;

  PN_stdfloat _fade_in;
  PN_stdfloat _fade_out;

  PN_stdfloat _num_frames;
  PN_stdfloat _frame_rate;

  typedef pvector<AnimEvent> AnimEvents;
  AnimEvents _events;

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
