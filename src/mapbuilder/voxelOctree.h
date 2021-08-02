/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file voxelOctree.h
 * @author brian
 * @date 2021-07-13
 */

#ifndef VOXELOCTREE_H
#define VOXELOCTREE_H

#include "pandabase.h"
#include "luse.h"
#include "boundingBox.h"
#include <list>

/**
 * An octree of solid voxel coordinates for fast look up.
 */
class EXPCL_PANDA_MAPBUILDER VoxelOctree {
PUBLISHED:
  class Node {
  public:
    Node() : children(), empty(true) { };
    int children[8];
    LPoint3 mins, maxs;
    LPoint3 center, half;
    bool empty;
    LPoint3i voxel;
    INLINE bool is_leaf() const { return children[0] == 0; }
  };

  INLINE VoxelOctree(BoundingBox *scene_bounds, const LVector3 &voxel_size, const LPoint3 &scene_mins);

  INLINE size_t get_num_nodes() const;
  INLINE const Node *get_node(size_t n) const;

  INLINE bool contains(const LPoint3i &voxel, size_t head_node = 0) const;

  INLINE int get_num_solid_leaves() const;

  INLINE int get_octant_containing_point(const Node *node, const LPoint3i &point) const;

  size_t get_lowest_node_containing_box(const LPoint3 &min, const LPoint3 &max) const;

  bool raycast(const LPoint3 &a, const LPoint3 &b, LPoint3i &voxel, size_t head_node = 0) const;

  void output(std::ostream &out) const;

  bool insert(const LPoint3i &voxel, size_t head_node = 0);

private:
  void r_output(const Node *node, std::ostream &out, int indent_level) const;
  void r_lowest_node_containing_box(const Node *node, const LPoint3 &min, const LPoint3 &max, size_t &lowest_index, int &lowest_depth, int depth) const;

public:
  pvector<Node> _nodes;
  LVector3 _voxel_size;
  LPoint3 _scene_min;
  int _num_solid_leaves;
};

#include "voxelOctree.I"

#endif // VOXELOCTREE_H
