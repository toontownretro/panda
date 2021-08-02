/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file voxelOctree.cxx
 * @author brian
 * @date 2021-07-13
 */

#include "voxelOctree.h"
#include "indent.h"
#include <stack>
#include <algorithm>

static LPoint3
mul_p3(const LPoint3 &a, const LPoint3 &b) {
  return LPoint3(
    a[0] * b[0],
    a[1] * b[1],
    a[2] * b[2]
  );
}

static bool
box_contains_lineseg(const LPoint3 &min, const LPoint3 &max, const LPoint3 &a, const LPoint3 &b) {
  if (a == b) {
    return (
      a[0] >= min[0] && a[0] <= max[0] &&
      a[1] >= min[1] && a[1] <= max[1] &&
      a[2] >= min[2] && a[2] <= max[2]
    );

  } else {
    // Set a bit for each plane a and b are on the wrong side of.
    unsigned int a_bits = 0;

    if (a[0] < min[0]) {
      a_bits |= 0x01;
    } else if (a[0] > max[0]) {
      a_bits |= 0x02;
    }

    if (a[1] < min[1]) {
      a_bits |= 0x04;
    } else if (a[1] > max[1]) {
      a_bits |= 0x08;
    }

    if (a[2] < min[2]) {
      a_bits |= 0x10;
    } else if (a[2] > max[2]) {
      a_bits |= 0x20;
    }

    unsigned int b_bits = 0;

    if (b[0] < min[0]) {
      b_bits |= 0x01;
    } else if (b[0] > max[0]) {
      b_bits |= 0x02;
    }

    if (b[1] < min[1]) {
      b_bits |= 0x04;
    } else if (b[1] > max[1]) {
      b_bits |= 0x08;
    }

    if (b[2] < min[2]) {
      b_bits |= 0x10;
    } else if (b[2] > max[2]) {
      b_bits |= 0x20;
    }

    if ((a_bits & b_bits) != 0) {
      // If there are any bits in common, the segment is wholly outside the
      // box (both points are on the wrong side of the same plane).
      return false;

    } else if ((a_bits | b_bits) == 0) {
      // If there are no bits at all, the segment is wholly within the box.
      return true;

    } else if (a_bits == 0 || b_bits == 0) {
      // If either point is within the box, the segment is partially within
      // the box.
      return true;

    } else {
      unsigned int differ = (a_bits ^ b_bits);
      if (differ == 0x03 || differ == 0x0c || differ == 0x30) {
        // If the line segment stretches straight across the box, the segment
        // is partially within.
        return true;
      } else {
        // Otherwise, it's hard to tell whether it does or doesn't.
        return false;
      }
    }
  }
}

/**
 * Inserts the indicated voxel into the tree.  Returns true if the voxel was
 * already in there.
 */
bool VoxelOctree::
insert(const LPoint3i &voxel, size_t head_node) {

  Node *n = &_nodes[head_node];
  while (true) {
    if (n->is_leaf()) {
      if (n->empty) {
        n->voxel = voxel;
        n->empty = false;
        _num_solid_leaves++;
        return false;

      } else {
        if (n->voxel == voxel) {
          // It's already in the tree.
          return true;
        }

        // Leaf already occupied, split it.
        LPoint3i old_data = n->voxel;

        _num_solid_leaves--;

        // Remember the parent index, pointer will be invalid after appending
        // to node array.
        size_t parent_index = n - _nodes.data();

        for (int i = 0; i < 8; i++) {
          LPoint3 new_center = n->center;
          new_center[0] += n->half[0] * (i & 4 ? 0.5 : -0.5);
          new_center[1] += n->half[1] * (i & 2 ? 0.5 : -0.5);
          new_center[2] += n->half[2] * (i & 1 ? 0.5 : -0.5);
          LVector3 new_half = n->half * 0.5;

          Node new_n;
          new_n.mins = new_center - new_half;
          new_n.maxs = new_center + new_half;
          new_n.half = new_half;
          new_n.center = new_center;
          _nodes.push_back(std::move(new_n));
          _nodes[parent_index].children[i] = _nodes.size();
          n = &_nodes[parent_index];
        }

        insert(old_data, parent_index);
        return insert(voxel, parent_index);
      }
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
 *
 */
void VoxelOctree::
output(std::ostream &out) const {
  const Node *n = &_nodes[0];
  r_output(n, out, 0);
}

/**
 *
 */
void VoxelOctree::
r_output(const Node *n, std::ostream &out, int indent_level) const {
  if (n->is_leaf()) {
    indent(out, indent_level) << "leaf";
  } else {
    indent(out, indent_level) << "node";
  }
  out << " mins " << n->mins << " maxs " << n->maxs;
  if (n->is_leaf()) {
    out << " value ";
    if (n->empty) {
      out << "empty";
    } else {
      out << n->voxel;
    }
  }
  out << "\n";

  for (int i = 0; i < 8; i++) {
    int child_idx = n->children[i];
    if (child_idx == 0) {
      continue;
    }
    const Node *child = &_nodes[child_idx - 1];
    r_output(child, out, indent_level + 2);
  }
}

/**
 * Returns the index of the lowest node in the tree that completely encloses
 * the indicated box.
 */
size_t VoxelOctree::
get_lowest_node_containing_box(const LPoint3 &mins, const LPoint3 &maxs) const {
  const Node *n = &_nodes[0];
  size_t lowest_index = 0;
  int lowest_depth = 0;
  r_lowest_node_containing_box(n, mins, maxs, lowest_index, lowest_depth, 0);
  return lowest_index;
}

/**
 *
 */
void VoxelOctree::
r_lowest_node_containing_box(const Node *n, const LPoint3 &mins, const LPoint3 &maxs,
                             size_t &lowest_index, int &lowest_depth, int depth) const {
  if (mins[0] >= n->mins[0] && maxs[0] <= n->maxs[0] &&
      mins[1] >= n->mins[1] && maxs[1] <= n->maxs[1] &&
      mins[2] >= n->mins[2] && maxs[2] <= n->maxs[2]) {
    // This node completely encloses the box.
    if (depth > lowest_depth) {
      lowest_index = n - _nodes.data();
      lowest_depth = depth;
    }

    // Check the children.
    for (int i = 0; i < 8; i++) {
      int child_idx = n->children[i];
      if (child_idx < 1) {
        continue;
      }
      r_lowest_node_containing_box(&_nodes[child_idx - 1], mins, maxs, lowest_index,
                                    lowest_depth, depth + 1);
    }
  }
}

/**
 * Casts the indicated ray through the octree.  Returns true if the ray
 * intersects a solid leaf, in which case the voxel coordinate is filled in.
 * If there are multiple solid leaves along the ray, the reported voxel is
 * not guaranteed to be the closest.  This only checks if there are *any*
 * solid leaves along the ray.
 */
bool VoxelOctree::
raycast(const LPoint3 &a, const LPoint3 &b, LPoint3i &voxel, size_t head_node) const {
  std::stack<int> stack;
  stack.push((int)(head_node) + 1);

  while (!stack.empty()) {
    int node_index = stack.top() - 1;
    stack.pop();

    if (node_index < 0) {
      continue;
    }

    const Node *n = &_nodes[node_index];
    LPoint3 node_mins = mul_p3(n->mins, _voxel_size) + _scene_min;
    LPoint3 node_maxs = node_mins + _voxel_size;
    if (box_contains_lineseg(node_mins, node_maxs, a, b)) {
      if (n->is_leaf()) {
        if (!n->empty) {
          LPoint3 leaf_mins = mul_p3(LPoint3(n->voxel[0], n->voxel[1], n->voxel[2]), _voxel_size) + _scene_min;
          LPoint3 leaf_maxs = leaf_mins + _voxel_size;
          if (box_contains_lineseg(leaf_mins, leaf_maxs, a, b)) {
            voxel = n->voxel;
            return true;
          }
        }

      } else {
        // Visit the node's children.
        int sorted_children[8];
        memcpy(sorted_children, n->children, sizeof(int) * 8);
        // Sort the children by distance from bbox center to start of ray.
        std::sort(sorted_children, sorted_children + 8,
                  [a, this](const int &child_index_a, const int &child_index_b) -> bool {
                    const Node *child_a = &_nodes[child_index_a - 1];
                    const Node *child_b = &_nodes[child_index_b - 1];
                    return (child_a->center - a).length_squared() < (child_b->center - a).length_squared();
                  });
        for (int i = 0; i < 8; i++) {
          stack.push(sorted_children[i]);
        }
      }
    }
  }

  return false;
}
