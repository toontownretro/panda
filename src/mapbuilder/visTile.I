/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visTile.I
 * @author brian
 * @date 2021-07-13
 */

/**
 * Returns the total number of voxels in the tile, solid or empty.
 */
INLINE int VisTile::
get_num_voxels() const {
  return (_max_voxel[0] - _min_voxel[0] + 1) *
         (_max_voxel[1] - _min_voxel[1] + 1) *
         (_max_voxel[2] - _min_voxel[2] + 1);
}

/**
 * Returns true if any areas part of this tile contain the indicated voxel,
 * false otherwise.
 */
INLINE bool VisTile::
contains_voxel(const LPoint3i &voxel) const {
  for (Area *area : _areas) {
    if (area->contains_voxel(voxel)) {
      return true;
    }
  }
  return false;
}
