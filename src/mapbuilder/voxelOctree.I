/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file voxelOctree.I
 * @author brian
 * @date 2021-07-13
 */

/**
 *
 */
INLINE VoxelOctree::
VoxelOctree(BoundingBox *scene_bounds, const LVector3 &voxel_size, const LPoint3 &scene_min)
{
  _voxel_size = voxel_size;
  _scene_min = scene_min;
  Node root;
  root.mins = scene_bounds->get_minq();
  root.maxs = scene_bounds->get_maxq();
  root.center = (root.maxs + root.mins) * 0.5;
  root.half = (root.maxs - root.mins) * 0.5;
  _nodes.push_back(root);
  _num_solid_leaves = 0;
}

/**
 * Returns the number of nodes in the tree.
 */
INLINE size_t VoxelOctree::
get_num_nodes() const {
  return _nodes.size();
}

/**
 * Returns the nth node of the tree.
 */
INLINE const VoxelOctree::Node *VoxelOctree::
get_node(size_t n) const {
  nassertr(n < _nodes.size(), nullptr);
  return &_nodes[n];
}

/**
 * Returns true if the indicated voxel exists in the tree, false otherwise.
 */
INLINE bool VoxelOctree::
contains(const LPoint3i &voxel, size_t head_node) const {
  const Node *n = &_nodes[head_node];
  while (true) {
    if (n->is_leaf()) {
      return !n->empty && n->voxel == voxel;

    } else {
      int child_idx = n->children[get_octant_containing_point(n, voxel)];
      if (child_idx < 1) {
        return false;
      }
      n = &_nodes[child_idx - 1];
    }
  }

  return false;
}

/**
 * Returns the number of leaves with stuff in them.
 */
INLINE int VoxelOctree::
get_num_solid_leaves() const {
  return _num_solid_leaves;
}

/**
 * Returns the index of the child of the indicated node that contains the
 * indicated point.
 */
INLINE int VoxelOctree::
get_octant_containing_point(const Node *node, const LPoint3i &point) const {
  int oct = 0;
  if (point[0] >= node->center[0]) {
    oct |= 4;
  }
  if (point[1] >= node->center[1]) {
    oct |= 2;
  }
  if (point[2] >= node->center[2]) {
    oct |= 1;
  }
  return oct;
}
