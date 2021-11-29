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
class EXPCL_PANDA_PGRAPH SceneVisibility : public TypedWritableReferenceCount, public WeakPointerCallback {
PUBLISHED:

  class NodeVisData : public ReferenceCount {
  public:
    // Relevant state of node at the time we last computed its vis sectors.
    // If any of these change, we have to recompute the node's vis sectors.
    CPT(TransformState) parent_net_transform;
    const BoundingVolume *node_bounds;

    // Set of visibility sectors that the node's external bounding volume
    // overlaps with.  AND'd with PVS during the cull traversal to determine
    // if node (and children) are in the PVS.
    BitArray vis_sectors;
    // Index of lowest node in K-D tree that completely encloses the Panda
    // node and all nodes below.  Allows children to shortcut K-D tree
    // traversal.
    int vis_head_node;
  };

  SceneVisibility();

  INLINE bool is_point_in_pvs(const LPoint3 &point, const BitArray &pvs) const;
  bool is_box_in_pvs(const LPoint3 &mins, const LPoint3 &maxs, const BitArray &pvs, int &lowest_node, int head_node = 0) const;
  bool is_sphere_in_pvs(const LPoint3 &center, PN_stdfloat radius, const BitArray &pvs, int &lowest_node, int head_node = 0) const;

  INLINE int get_point_sector(const LPoint3 &point) const;
  void get_box_sectors(const LPoint3 &mins, const LPoint3 &maxs, BitArray &sectors, int &lowest_node, int head_node) const;
  void get_sphere_sectors(const LPoint3 &center, PN_stdfloat radius, BitArray &sectors, int &lowest_node, int head_node) const;

  INLINE void add_sector_pvs(const BitArray &pvs);
  INLINE int get_num_sectors() const;
  INLINE const BitArray *get_sector_pvs(int sector) const;

  INLINE void set_sector_tree(const KDTree &tree);
  INLINE const KDTree *get_sector_tree() const;

  int is_node_in_pvs(const NodePath &node, const BitArray &pvs, const BitArray &inv_pvs);
  int is_node_in_pvs(const TransformState *parent_net_transform,
                     const GeometricBoundingVolume *bounds,
                     PandaNode *node, const BitArray &pvs, const BitArray &inv_pvs,
                     int &lowest_kd_node, int head_node);
  NodeVisData *get_node_vis(PandaNode *node);
  INLINE void clear_node_vis_cache();

public:
  virtual void wp_callback(void *data) override;

private:
  // Spatial search structure used to find sectors from points and volumes
  // in space.
  KDTree _sector_tree;
  // PVS for each sector.  Each bit in the BitArray corresponds to a sector
  // index, and a 1 bit means that sector is potentially visible from the
  // other one.  The bit corresponding to the sector itself is also set for
  // simplicity.
  pvector<BitArray> _sector_pvs;

  typedef pflat_hash_map<PandaNode *, PT(NodeVisData), pointer_hash> NodeVisCache;
  NodeVisCache _node_vis_cache;

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
