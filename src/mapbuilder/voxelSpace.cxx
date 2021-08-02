/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file voxelSpace.cxx
 * @author brian
 * @date 2021-07-10
 */

#include "voxelSpace.h"

static int
closest_pow2_up(const LVecBase3i &counts) {
  int max = INT_MIN;
  max = std::max(counts[0], max);
  max = std::max(counts[1], max);
  max = std::max(counts[2], max);

  int power = 1;
  while (power < max) {
    power <<= 1;
  }

  return power;
}

/**
 *
 */
VoxelSpace::
VoxelSpace() :
  _solid_voxels(new BoundingBox, 1, 0)
{
}

/**
 *
 */
VoxelSpace::
VoxelSpace(const LVecBase3 &voxel_size, const LVecBase3i &counts, BoundingBox *scene_bbox) :
  _solid_voxels(new BoundingBox(LPoint3(0), LPoint3(counts[0], counts[1], counts[2])), voxel_size, scene_bbox->get_minq()) {
  _scene_bbox = scene_bbox;
  _voxel_counts = counts;
  _voxel_size = voxel_size;
}

/**
 * Returns a bounding box that encloses the indicated range of voxels.
 */
PT(BoundingBox) VoxelSpace::
get_voxel_bounds(const LPoint3i &min_voxel, const LPoint3i &max_voxel) const {
  //nassertr(is_valid_voxel_coord(min_voxel) && is_valid_voxel_coord(max_voxel), new BoundingBox);

  LPoint3 min_point(
    (PN_stdfloat)min_voxel[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    (PN_stdfloat)min_voxel[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    (PN_stdfloat)min_voxel[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]);
  LPoint3 max_point = LPoint3(
    (PN_stdfloat)max_voxel[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    (PN_stdfloat)max_voxel[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    (PN_stdfloat)max_voxel[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]) + _voxel_size;
  return new BoundingBox(min_point, max_point);
}

/**
 *
 */
void VoxelSpace::
get_voxel_bounds(const LPoint3i &min_voxel, const LPoint3i &max_voxel,
                 LPoint3 &mins, LPoint3 &maxs) const {
  mins.set(
    (PN_stdfloat)min_voxel[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    (PN_stdfloat)min_voxel[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    (PN_stdfloat)min_voxel[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]);
  maxs.set(
    (PN_stdfloat)max_voxel[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    (PN_stdfloat)max_voxel[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    (PN_stdfloat)max_voxel[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]);
  maxs += _voxel_size;
}

/**
 * Returns the bounding box of the indicated voxel.
 */
PT(BoundingBox) VoxelSpace::
get_voxel_bounds(const LPoint3i &voxel_coord) const {
  nassertr(is_valid_voxel_coord(voxel_coord), new BoundingBox);

  LPoint3 min_point(
    (PN_stdfloat)voxel_coord[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    (PN_stdfloat)voxel_coord[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    (PN_stdfloat)voxel_coord[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]
  );
  return new BoundingBox(min_point, min_point + _voxel_size);
}

/**
 * Returns the bounding box of the indicated voxel as a min and max point.
 */
void VoxelSpace::
get_voxel_bounds(const LPoint3i &voxel_coord, LPoint3 &mins, LPoint3 &maxs) const {
  nassertv(is_valid_voxel_coord(voxel_coord));
  mins.set(
    (PN_stdfloat)voxel_coord[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    (PN_stdfloat)voxel_coord[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    (PN_stdfloat)voxel_coord[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]);
  maxs = mins + _voxel_size;
}

/**
 * Returns the center point of the indicated voxel.
 */
LPoint3 VoxelSpace::
get_voxel_center(const LPoint3i &voxel_coord) const {
  nassertr(is_valid_voxel_coord(voxel_coord), LPoint3());

  LPoint3 min_point(
    voxel_coord[0] * _voxel_size[0] + _scene_bbox->get_minq()[0],
    voxel_coord[1] * _voxel_size[1] + _scene_bbox->get_minq()[1],
    voxel_coord[2] * _voxel_size[2] + _scene_bbox->get_minq()[2]
  );
  return min_point + (_voxel_size * 0.5f);
}

/**
 * Returns a list of bounding boxes for each voxel contained within the
 * indicated bounding box.
 */
pvector<PT(BoundingBox)> VoxelSpace::
get_voxel_bounds_within(BoundingBox *bounds) const {
  LPoint3i from_coord = get_voxel_coord(bounds->get_minq());
  LPoint3i to_coord = get_voxel_coord(bounds->get_maxq());

  pvector<PT(BoundingBox)> voxel_bounds;

  // Calculate number of voxels within the bounds.
  size_t total = (to_coord[0] - from_coord[0] + 1) *
              (to_coord[1] - from_coord[1] + 1) *
              (to_coord[2] - from_coord[2] + 1);
  voxel_bounds.reserve(total);

  for (int z = from_coord[2]; z <= to_coord[2]; z++) {
    for (int y = from_coord[1]; y <= to_coord[1]; y++) {
      for (int x = from_coord[0]; x <= to_coord[0]; x++) {
        voxel_bounds.push_back(get_voxel_bounds(LPoint3i(x, y, z)));
      }
    }
  }

  return voxel_bounds;
}

/**
 * Returns a list of voxel coordinates in the indicated range.
 */
pvector<LPoint3i> VoxelSpace::
get_voxel_coords_in_range(const LPoint3i &from_coord, const LPoint3i &to_coord) const {
  pvector<LPoint3i> voxel_coords;
  // Calculate number of voxels within the bounds.
  size_t total = (to_coord[0] - from_coord[0] + 1) *
              (to_coord[1] - from_coord[1] + 1) *
              (to_coord[2] - from_coord[2] + 1);
  voxel_coords.reserve(total);

  for (int z = from_coord[2]; z <= to_coord[2]; z++) {
    for (int y = from_coord[1]; y <= to_coord[1]; y++) {
      for (int x = from_coord[0]; x <= to_coord[0]; x++) {
        voxel_coords.push_back(LPoint3i(x, y, z));
      }
    }
  }

  return voxel_coords;
}

/**
 * Returns the coordinate of the voxel that neighbors the indicated voxel
 * in the indicated direction.
 */
LPoint3i VoxelSpace::
get_voxel_neighbor(const LPoint3i &voxel_coord, NeighborDirection dir) const {
  nassertr(is_valid_voxel_coord(voxel_coord), LPoint3i());

  LPoint3i coord = voxel_coord;
  switch (dir) {
  default:
  case ND_front:
    coord[1] += 1;
    break;
  case ND_back:
    coord[1] -= 1;
    break;
  case ND_left:
    coord[0] -= 1;
    break;
  case ND_right:
    coord[0] += 1;
    break;
  case ND_down:
    coord[2] -= 1;
    break;
  case ND_up:
    coord[2] += 1;
    break;
  }

  return coord;
}
