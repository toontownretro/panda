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
#include "nodePath.h"

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

  /**
   * An event that occurs somewhere in the timeline of an AnimChannel.  Note
   * that events are only processed on the top-level channel.
   */
  class EXPCL_PANDA_ANIM Event {
  PUBLISHED:
    INLINE Event(int type, int event, PN_stdfloat cycle,
                 const std::string &options) :
      _type(type),
      _cycle(cycle),
      _event(event),
      _options(options)
    {}

    INLINE Event() :
      _type(0),
      _cycle(0),
      _event(0)
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

    friend class AnimChannel;
  };

  /**
   * Defines an IK operation for an AnimChannel for a particular IK chain of
   * a character.
   */
  class EXPCL_PANDA_ANIM IKEvent {
  PUBLISHED:
    enum Type {
      T_invalid = -1,
      // Maintain original chain position after AnimChannel is applied.
      T_lock,
      // Move chain relative to another joint on the character.
      T_touch,
      // Move chain to user-specified world-space target position.
      T_target,
      // Remove global IK events added by channels above this one.
      T_release,
    };

    class TouchOffset {
    public:
      LPoint3 _pos;
      LVecBase3 _hpr;
    };

    IKEvent() = default;

    Type _type = T_invalid;
    int _chain = -1;

    int _touch_joint = -1;
    pvector<TouchOffset> _touch_offsets;

    // Blending parameters.
    PN_stdfloat _start = 0.0f;
    PN_stdfloat _peak = 0.0f;
    PN_stdfloat _tail = 0.0f;
    PN_stdfloat _end = 0.0f;
    bool _spline = false;
    // If >= 0, blending controlled by pose parameter value instead of
    // anim cycle.
    int _pose_parameter = -1;
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

  void add_event(int type, int event, PN_stdfloat frame,
                 const std::string &options = "");
  INLINE int get_num_events() const;
  INLINE const Event &get_event(int n) const;

  void add_ik_event(const IKEvent &event);
  INLINE int get_num_ik_events() const;
  INLINE const IKEvent *get_ik_event(int n) const;

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

  void calc_pose(const AnimEvalContext &context, AnimEvalData &data);

  /**
   * Computes the pose of each joint for this channel in the given context.
   */
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data)=0;

  void blend(const AnimEvalContext &context, AnimEvalData &a,
             AnimEvalData &b, PN_stdfloat weight) const;

  virtual LVector3 get_root_motion_vector(Character *character) const=0;

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

  typedef pvector<Event> Events;
  Events _events;

  typedef pvector<IKEvent> IKEvents;
  IKEvents _ik_events;
};

#include "animChannel.I"

#endif // ANIMCHANNEL_H
