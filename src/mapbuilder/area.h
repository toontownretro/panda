/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file area.h
 * @author brian
 * @date 2021-07-13
 */

#ifndef AREA_H
#define AREA_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "portal.h"
#include "luse.h"
#include "voxelSpace.h"

class VisBuilder;

/**
 *
 */
class EXPCL_PANDA_MAPBUILDER Area : public ReferenceCount {
public:
  Area();

  INLINE bool contains_voxel(const LPoint3i &voxel) const;

  LPoint3i _min_voxel;
  LPoint3i _max_voxel;

  pvector<PT(Portal)> _portals; // Portals connecting this area to other areas.
  //pvector<PT(Area)> _visible_areas; // Other areas visible from this area.

  bool _is_cluster;

  int _group;
};

/**
 * A cluster of areas.
 */
class EXPCL_PANDA_MAPBUILDER AreaCluster : public Area {
public:
  AreaCluster();

  /**
   * The voxel bounds of an area contained in the cluster.
   */
  class AreaBounds {
  public:
    LPoint3i _min_voxel, _max_voxel;
  };
  pvector<AreaBounds> _contained_areas;

  pvector<AreaBounds> _cluster_boxes;

  int _id;

  // True if an entity can reach the cluster.  If false, the cluster is
  // outside the world and can be removed.
  bool _occupied;

  // Path the occupant took to reach this cluster.
  pvector<LPoint3> _occupied_path;

  // Indices of potentially visible clusters.
  pset<int> _pvs;

  void add_area(Area *area);

  LPoint3i get_area_seed_point() const;
  bool boxes_contain_voxel(const LPoint3i &voxel) const;
  bool box_intersects_existing_boxes(const LPoint3i &min, const LPoint3i &max) const;

  void test_expansion(LPoint3i &min, LPoint3i &max,
                      VoxelSpace::NeighborDirection dir, VisBuilder *vis) const;
  bool test_box(LPoint3i &min, LPoint3i &max, VoxelSpace::NeighborDirection dir,
                VisBuilder *vis) const;
};

#include "area.I"

#endif // AREA_H
