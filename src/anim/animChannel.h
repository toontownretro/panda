/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannel.h
 * @author brian
 * @date 2021-08-04
 */

#ifndef ANIMCHANNEL_H
#define ANIMCHANNEL_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "weightList.h"
#include "animEvalContext.h"
#include "namable.h"

class Character;

/**
 * This is an abstract base class for all types of animation channels.
 */
class EXPCL_PANDA_ANIM AnimChannel : public TypedWritableReferenceCount, public Namable {
  DECLARE_CLASS(AnimChannel, TypedWritableReferenceCount);

PUBLISHED:
  enum Flags {
    F_none = 0,

    // The channel should be blended additively.
    F_delta = 1 << 0,
    // The channel should be blended additively in an underlay fashion.
    F_pre_delta = 1 << 1,

    // Override X value of root joint with zero.
    F_zero_root_x = 1 << 2,
    // Override Y value of root joint with zero.
    F_zero_root_y = 1 << 3,
    // Override Z value of root joint with zero.
    F_zero_root_z = 1 << 4,

    // Channel is looping.  Only applies to top level channel.
    F_looping = 1 << 5,

    // Don't blend transitions into other channels.  Only applies to top level
    // channel.
    F_snap = 1 << 6,

    // Cycle of channel is relative to global clock instead of animation start
    // time.
    F_real_time = 1 << 7,
  };

  AnimChannel(const std::string &name);

  /**
   * Creates and returns a copy of this AnimChannel.
   */
  virtual PT(AnimChannel) make_copy() const=0;

  /**
   * Returns the duration of the channel.
   */
  virtual PN_stdfloat get_length(Character *character) const=0;

  INLINE void set_num_frames(int count);
  INLINE int get_num_frames() const;

  INLINE void set_frame_rate(PN_stdfloat fps);
  INLINE PN_stdfloat get_frame_rate() const;

  INLINE PN_stdfloat get_cycle_rate(Character *character) const;

  INLINE void set_flags(unsigned int flags);
  INLINE bool has_flags(unsigned int flags) const;
  INLINE unsigned int get_flags() const;
  INLINE void clear_flags(unsigned int flags);

  INLINE void add_activity(int activity, PN_stdfloat weight = 1.0f);
  INLINE int get_num_activities() const;
  INLINE int get_activity(int n) const;
  INLINE PN_stdfloat get_activity_weight(int n) const;

  INLINE void set_weight_list(WeightList *list);
  INLINE WeightList *get_weight_list() const;

  INLINE void set_fade_in(PN_stdfloat time);
  INLINE PN_stdfloat get_fade_in() const;

  INLINE void set_fade_out(PN_stdfloat time);
  INLINE PN_stdfloat get_fade_out() const;

  /**
   * Computes the pose of each joint for this channel in the given context.
   */
  virtual void calc_pose(const AnimEvalContext &context, AnimEvalData &data)=0;

  void blend(const AnimEvalContext &context, AnimEvalData &a,
             const AnimEvalData &b, PN_stdfloat weight) const;

public:
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;
  virtual void write_datagram(BamWriter *manager, Datagram &me) override;

protected:
  AnimChannel(const AnimChannel &copy);
  AnimChannel();

  void fillin(DatagramIterator &scan, BamReader *manager);

protected:
  int _num_frames;
  PN_stdfloat _fps;

  struct ActivityDef {
    int activity;
    PN_stdfloat weight;
  };
  typedef pvector<ActivityDef> Activities;
  Activities _activities;

  unsigned int _flags;

  PN_stdfloat _fade_in;
  PN_stdfloat _fade_out;

  // Controls per-joint weighting of the evaluated pose for the channel.
  PT(WeightList) _weights;
};

#include "animChannel.I"

#endif // ANIMCHANNEL_H
