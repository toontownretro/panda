/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBlend1D.h
 * @author brian
 * @date 2021-08-04
 */

#ifndef ANIMCHANNELBLEND1D_H
#define ANIMCHANNELBLEND1D_H

#include "pandabase.h"
#include "animChannel.h"
#include "pset.h"

/**
 * This is an AnimChannel that is composed of several nested AnimChannels.
 * The channels are composited by blending between them using a linear 1-D
 * blend space.
 */
class EXPCL_PANDA_ANIM AnimChannelBlend1D final : public AnimChannel {
  DECLARE_CLASS(AnimChannelBlend1D, AnimChannel);

PUBLISHED:
  AnimChannelBlend1D(const std::string &name);

  virtual PT(AnimChannel) make_copy() const override;

  INLINE void set_blend_param(int param);
  INLINE int get_blend_param() const;

  void add_channel(AnimChannel *channel, PN_stdfloat coord);

  virtual PN_stdfloat get_length(Character *character) const override;
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &data) override;

public:
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

protected:
  AnimChannelBlend1D(const AnimChannelBlend1D &copy);

  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  int _blend_param;

  class Channel {
  public:
    class Compare {
    public:
      bool operator () (const Channel &a, const Channel &b) const {
        return a._blend_coord < b._blend_coord;
      }
    };

    PT(AnimChannel) _channel;
    PN_stdfloat _blend_coord;
  };
  typedef pvector<Channel> Channels;
  Channels _channels;

  bool _sorted;

private:
  PN_stdfloat get_blend_targets(Character *character, const Channel *&from, const Channel *&to);
};

#include "animChannelBlend1D.I"

#endif // ANIMCHANNELBLEND1D_H
