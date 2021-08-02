/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file voxelSpace.h
 * @author brian
 * @date 2021-07-10
 */

#ifndef VOXELSPACE_H
#define VOXELSPACE_H

#include "pandabase.h"
#include "luse.h"
#include "pset.h"
#include "pmap.h"
#include "boundingBox.h"
#include "voxelOctree.h"

/**
 * A space of voxels.
 */
class EXPCL_PANDA_MAPBUILDER VoxelSpace {
public:
  enum NeighborDirection {
    ND_front,
    ND_back,
    ND_left,
    ND_right,
    ND_down,
    ND_up,
  };

  enum VoxelType : uint8_t {
    VT_solid,
    VT_empty,
  };

  VoxelSpace();
  VoxelSpace(const LVecBase3 &voxel_size, const LVecBase3i &counts, BoundingBox *scene_bbox);

  INLINE void set_voxel_type(const LPoint3i &voxel_coord, VoxelType type);
  INLINE VoxelType get_voxel_type(const LPoint3i &voxel_coord) const;

  PT(BoundingBox) get_voxel_bounds(const LPoint3i &min_voxel, const LPoint3i &max_voxel) const;
  PT(BoundingBox) get_voxel_bounds(const LPoint3i &voxel_coord) const;
  void get_voxel_bounds(const LPoint3i &min_voxel, const LPoint3i &max_voxel,
                        LPoint3 &mins, LPoint3 &maxs) const;
  void get_voxel_bounds(const LPoint3i &voxel_coord, LPoint3 &mins, LPoint3 &maxs) const;
  LPoint3 get_voxel_center(const LPoint3i &voxel_coord) const;

  INLINE LPoint3i get_voxel_coord(const LPoint3 &world_coord) const;

  INLINE size_t get_num_solid_voxels() const;
  INLINE pvector<LPoint3i> get_solid_voxels() const;
  INLINE pvector<LPoint3> get_solid_voxel_centers() const;

  pvector<PT(BoundingBox)> get_voxel_bounds_within(BoundingBox *bounds) const;
  pvector<LPoint3i> get_voxel_coords_in_range(const LPoint3i &mins, const LPoint3i &maxs) const;

  INLINE size_t get_num_voxels() const;
  INLINE const LVecBase3i &get_voxel_counts() const;
  INLINE const LVecBase3 &get_voxel_size() const;

  INLINE BoundingBox *get_scene_bounds() const;

  INLINE bool is_valid_voxel_coord(const LPoint3i &voxel_coord) const;

  LPoint3i get_voxel_neighbor(const LPoint3i &voxel_coord, NeighborDirection dir) const;

public:
  LVecBase3i _voxel_counts;
  LVecBase3 _voxel_size;

  VoxelOctree _solid_voxels;

  PT(BoundingBox) _scene_bbox;
};

#include "voxelSpace.I"

#endif // VOXELSPACE_H
