/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneVisibility.h
 * @author brian
 * @date 2021-11-17
 */

#ifndef SCENEVISIBILITY_H
#define SCENEVISIBILITY_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "bitArray.h"
#include "luse.h"
#include "kdTree.h"
#include "pmap.h"
#include "weakPointerTo.h"
#include "weakPointerCallback.h"
#include "pandaNode.h"
#include "referenceCount.h"
#include "pointerTo.h"

class TransformState;
class BoundingVolume;

/**
 * This object contains pre-computed visibility information for a scene.
 * It is stored in the SceneTop node, and if present, is utilized by the
 * CullTraverser to cull nodes that are not potentially visibile, along with
 * the normal view-frustum test.
 *
 * The visibility information partitions the world into distinct sectors with
 * unique IDs, and stores a potentially visible set between all sectors.
 * That is, a list of sector IDs that are potentially visible from another
 * sector.
 *
 * The object provides an interface to efficiently query the visibility
 * sector(s) that a point or volume in space overlaps with, as well as
 * an interface to query the potentially visible set of a given sector.
 *
 * This information can be utilized for more than just rendering culling.
 * For instance, it can also be used for network culling, such as not
 * transmitting network state of an object to a client that is not in
 * the potentially visible set of the sector that the client's camera is
 * in.
 */
class EXPCL_PANDA_PGRAPH SceneVisibility : public TypedWritableReferenceCount {
PUBLISHED:

  SceneVisibility();

  INLINE bool is_point_in_pvs(const LPoint3 &point, const BitArray &pvs) const;
  bool is_box_in_pvs(const LPoint3 &mins, const LPoint3 &maxs, const BitArray &pvs, int &lowest_node, int head_node = 0) const;
  bool is_sphere_in_pvs(const LPoint3 &center, PN_stdfloat radius, const BitArray &pvs, int &lowest_node, int head_node = 0) const;

  INLINE int get_point_sector(const LPoint3 &point) const;
  void get_box_sectors(const LPoint3 &mins, const LPoint3 &maxs, int *sectors, int &num_sectors) const;
  void get_sphere_sectors(const LPoint3 &center, PN_stdfloat radius, int *sectors, int &num_sectors) const;

  INLINE void add_sector_pvs(const BitArray &pvs);
  INLINE int get_num_sectors() const;
  INLINE const BitArray *get_sector_pvs(int sector) const;

  INLINE void set_sector_tree(const KDTree &tree);
  INLINE const KDTree *get_sector_tree() const;

private:
  // Spatial search structure used to find sectors from points and volumes
  // in space.
  KDTree _sector_tree;
  // PVS for each sector.  Each bit in the BitArray corresponds to a sector
  // index, and a 1 bit means that sector is potentially visible from the
  // other one.  The bit corresponding to the sector itself is also set for
  // simplicity.
  pvector<BitArray> _sector_pvs;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "SceneVisibility",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sceneVisibility.I"

#endif // SCENEVISIBILITY_H
