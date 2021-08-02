/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file area.I
 * @author brian
 * @date 2021-07-13
 */

/**
 * Returns true if the area contains the indicated voxel, false otherwise.
 */
INLINE bool Area::
contains_voxel(const LPoint3i &voxel) const {
  return (
    voxel[0] >= _min_voxel[0] && voxel[0] <= _max_voxel[0] &&
    voxel[1] >= _min_voxel[1] && voxel[1] <= _max_voxel[1] &&
    voxel[2] >= _min_voxel[2] && voxel[2] <= _max_voxel[2]
  );
}
