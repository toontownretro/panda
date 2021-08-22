/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBundle.h
 * @author brian
 * @date 2021-08-05
 */

#ifndef ANIMCHANNELBUNDLE_H
#define ANIMCHANNELBUNDLE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "pvector.h"
#include "animChannel.h"

/**
 * A node that contains a collection of AnimChannels.  Like CharacterNode, it
 * exists solely to make it easy to store AnimChannels in the scene graph.
 */
class EXPCL_PANDA_ANIM AnimChannelBundle final : public PandaNode {
  DECLARE_CLASS(AnimChannelBundle, PandaNode);

PUBLISHED:
  INLINE AnimChannelBundle(const std::string &name);

  INLINE void add_channel(AnimChannel *channel);
  INLINE AnimChannel *get_channel(int n) const;
  INLINE int get_num_channels() const;

  virtual PandaNode *make_copy() const override;
  virtual bool safe_to_flatten() const override;

private:
  typedef pvector<PT(AnimChannel)> Channels;
  Channels _channels;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator& scan, BamReader* manager);
};

#include "animChannelBundle.I"

#endif // ANIMCHANNELBUNDLE_H
