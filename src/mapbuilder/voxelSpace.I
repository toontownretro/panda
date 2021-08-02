/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file voxelSpace.I
 * @author brian
 * @date 2021-07-10
 */

/**
 * Sets whether or not the indicated voxel is solid or empty.
 */
INLINE void VoxelSpace::
set_voxel_type(const LPoint3i &voxel_coord, VoxelType type) {
  nassertv(is_valid_voxel_coord(voxel_coord));
  if (type == VT_solid) {
    _solid_voxels.insert(voxel_coord);
  }
}

/**
 * Returns whether or not the indicated voxel is solid or empty.
 */
INLINE VoxelSpace::VoxelType VoxelSpace::
get_voxel_type(const LPoint3i &voxel_coord) const {
  nassertr(is_valid_voxel_coord(voxel_coord), VT_solid);
  if (_solid_voxels.contains(voxel_coord)) {
    return VT_solid;

  } else {
    return VT_empty;
  }
}

/**
 * Returns the coordinate of the voxel that contains the indicated world
 * coordinate.
 */
INLINE LPoint3i VoxelSpace::
get_voxel_coord(const LPoint3 &world_coord) const {
  return LPoint3i(
    (int)((world_coord[0] - _scene_bbox->get_minq()[0]) / _voxel_size[0]),
    (int)((world_coord[1] - _scene_bbox->get_minq()[1]) / _voxel_size[1]),
    (int)((world_coord[2] - _scene_bbox->get_minq()[2]) / _voxel_size[2])
  );
}

/**
 * Returns the number of solid voxels.
 */
INLINE size_t VoxelSpace::
get_num_solid_voxels() const {
  return _solid_voxels.get_num_solid_leaves();
}

/**
 * Returns all of the solid voxel coordinates.
 */
INLINE pvector<LPoint3i> VoxelSpace::
get_solid_voxels() const {
  pvector<LPoint3i> voxels;
  voxels.reserve(_solid_voxels.get_num_solid_leaves());
  for (size_t i = 0; i < _solid_voxels.get_num_nodes(); i++) {
    const VoxelOctree::Node *node = _solid_voxels.get_node(i);
    if (node->is_leaf() && !node->empty) {
      voxels.push_back(node->voxel);
    }
  }
  return voxels;
}

/**
 * Returns the center coordinate of each solid voxel.
 */
INLINE pvector<LPoint3> VoxelSpace::
get_solid_voxel_centers() const {
  pvector<LPoint3> centers;
  centers.reserve(_solid_voxels.get_num_solid_leaves());
  for (size_t i = 0; i < _solid_voxels.get_num_nodes(); i++) {
    const VoxelOctree::Node *node = _solid_voxels.get_node(i);
    if (node->is_leaf() && !node->empty) {
      centers.push_back(get_voxel_center(node->voxel));
    }
  }
  return centers;
}

/**
 * Returns the total number of voxels.
 */
INLINE size_t VoxelSpace::
get_num_voxels() const {
  return _voxel_counts[0] * _voxel_counts[1] * _voxel_counts[2];
}

/**
 * Returns the number of voxels in each direction.
 */
INLINE const LVecBase3i &VoxelSpace::
get_voxel_counts() const {
  return _voxel_counts;
}

/**
 * Returns the size of each voxel.
 */
INLINE const LVecBase3 &VoxelSpace::
get_voxel_size() const {
  return _voxel_size;
}

/**
 * Returns the bounding box of the entire space.
 */
INLINE BoundingBox *VoxelSpace::
get_scene_bounds() const {
  return _scene_bbox;
}

/**
 * Returns true if the indicated voxel coordinate is valid, false otherwise.
 */
INLINE bool VoxelSpace::
is_valid_voxel_coord(const LPoint3i &voxel_coord) const {
  return (voxel_coord[0] >= 0) && (voxel_coord[0] < _voxel_counts[0]) &&
         (voxel_coord[1] >= 0) && (voxel_coord[1] < _voxel_counts[1]) &&
         (voxel_coord[2] >= 0) && (voxel_coord[2] < _voxel_counts[2]);
}
