/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBlend2D.h
 * @author brian
 * @date 2021-08-04
 */

#ifndef ANIMCHANNELBLEND2D_H
#define ANIMCHANNELBLEND2D_H

#include "pandabase.h"
#include "animChannel.h"

/**
 * This is an AnimChannel that is composed of several nested AnimChannels.
 * The channels are composited by blending between them using a 2-D blend
 * space.
 */
class EXPCL_PANDA_ANIM AnimChannelBlend2D final : public AnimChannel {
  DECLARE_CLASS(AnimChannelBlend2D, AnimChannel);

PUBLISHED:
  AnimChannelBlend2D(const std::string &name);

  void build_triangles();
  void compute_weights();
  void compute_weights_if_necessary(Character *character);

  virtual PT(AnimChannel) make_copy() const override;

  virtual PN_stdfloat get_length(Character *character) const override;
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data) override;

  INLINE void set_blend_x(int param);
  INLINE int get_blend_x() const;

  INLINE void set_blend_y(int param);
  INLINE int get_blend_y() const;

  INLINE void add_channel(AnimChannel *channel, const LPoint2 &coord);
  INLINE int get_num_channels() const;
  INLINE AnimChannel *get_channel(int n) const;
  INLINE LPoint2 get_channel_coord(int n) const;

private:
  void blend_triangle(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c,
                      const LPoint2 &point, PN_stdfloat *weights);
  bool point_in_triangle(const LPoint2 &a, const LPoint2 &b,
                         const LPoint2 &c, const LPoint2 &point) const;
  PN_stdfloat triangle_sign(const LPoint2 &a, const LPoint2 &b,
                            const LPoint2 &c) const;
  LPoint2 closest_point_to_segment(const LPoint2 &point, const LPoint2 &a,
                                   const LPoint2 &b) const;

public:
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

protected:
  AnimChannelBlend2D(const AnimChannelBlend2D &copy);

  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  int _blend_x;
  int _blend_y;

  class Triangle {
  public:
    // Control point indices.
    int a;
    int b;
    int c;
  };
  typedef pvector<Triangle> Triangles;
  Triangles _triangles;
  bool _has_triangles;
  Triangle *_active_tri;

  LPoint2 _input_coord;

  class Channel {
  public:
    PT(AnimChannel) _channel;
    LPoint2 _point;
    PN_stdfloat _weight;
  };
  typedef pvector<Channel> Channels;
  Channels _channels;
};

#include "animChannelBlend2D.I"

#endif // ANIMCHANNELBLEND2D_H
