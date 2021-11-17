/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRoot.h
 * @author brian
 * @date 2021-07-09
 */

#ifndef MAPROOT_H
#define MAPROOT_H

#include "pandabase.h"
#include "pandaNode.h"
#include "mapData.h"

/**
 * The parent node of static geometry in a map.  It contains a static table
 * of area clusters to child node indices, so static map geometry can be very
 * quickly culled instead of having to test the bounding volumes against
 * the area cluster tree, which is done for dynamic nodes.
 */
class EXPCL_PANDA_MAP MapRoot : public PandaNode {
  DECLARE_CLASS(MapRoot, PandaNode);

PUBLISHED:
  MapRoot(MapData *data);

  INLINE MapData *get_data() const;

  INLINE void set_pvs_cull(bool flag);
  INLINE bool get_pvs_cull() const;

public:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;

  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

  static TypedWritable *make_from_bam(const FactoryParams &params);

  virtual PandaNode *make_copy() const override;

private:
  MapRoot();
  MapRoot(const MapRoot &copy);

private:
  PT(MapData) _data;
  bool _pvs_cull;
};

#include "mapRoot.I"

#endif // MAPROOT_H
