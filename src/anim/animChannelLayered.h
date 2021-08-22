/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelLayered.h
 * @author brian
 * @date 2021-08-05
 */

#ifndef ANIMCHANNELLAYERED_H
#define ANIMCHANNELLAYERED_H

#include "pandabase.h"
#include "animChannel.h"

/**
 * An AnimChannel that is composed of several nested AnimChannel layers.
 */
class EXPCL_PANDA_ANIM AnimChannelLayered final : public AnimChannel {
  DECLARE_CLASS(AnimChannelLayered, AnimChannel);

PUBLISHED:
  AnimChannelLayered(const std::string &name);

  void add_channel(AnimChannel *channel, PN_stdfloat start_frame = 0, PN_stdfloat peak_frame = 0,
                   PN_stdfloat tail_frame = 0, PN_stdfloat end_frame = 0, bool spline = false,
                   bool no_blend = false, bool xfade = false, int pose_param = -1);
  INLINE int get_num_channels() const;
  INLINE AnimChannel *get_channel(int n) const;

  virtual PT(AnimChannel) make_copy() const override;

  virtual PN_stdfloat get_length(Character *character) const override;

  virtual void calc_pose(const AnimEvalContext &context, AnimEvalData &data) override;

public:
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

protected:
  AnimChannelLayered(const AnimChannelLayered &copy);

  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  class Channel {
  public:
    PT(AnimChannel) _channel;
    PN_stdfloat _start;
    PN_stdfloat _peak;
    PN_stdfloat _tail;
    PN_stdfloat _end;
    bool _spline;
    bool _no_blend;
    bool _xfade;
    int _pose_parameter;
  };
  typedef pvector<Channel> Channels;
  Channels _channels;
};

#include "animChannelLayered.I"

#endif // ANIMCHANNELLAYERED_H
