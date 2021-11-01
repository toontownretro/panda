/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file kdTree.cxx
 * @author brian
 * @date 2021-07-24
 */

#include "kdTree.h"
#include "vector_stdfloat.h"
#include "geometricBoundingVolume.h"
#include "boundingPlane.h"
#include "indent.h"
#include "boundingBox.h"
#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "boundingLine.h"
#include "boundingPlane.h"

#include <algorithm>
#include <stack>

#define INVALID_NODE INT_MAX

/**
 *
 */
KDTree::
KDTree(KDTree &&other) :
  _nodes(std::move(other._nodes)),
  _leaves(std::move(other._leaves)),
  _inputs(std::move(other._inputs))
{
}

/**
 *
 */
void KDTree::
operator = (KDTree &&other) {
  _nodes = std::move(other._nodes);
  _leaves = std::move(other._leaves);
  _inputs = std::move(other._inputs);
}

/**
 * Builds the k-d tree from the set of input objects.
 */
void KDTree::
build() {
  _nodes.clear();
  _leaves.clear();

  _leaves.reserve(_inputs.size());
  _nodes.reserve(_inputs.size() * 2);

  // Start with the root node then split.
  vector_int objects;
  for (size_t i = 0; i < _inputs.size(); i++) {
    objects.push_back(i);
  }
  make_subtree(objects);

  _inputs.clear();
}

/**
 * Clears the whole tree.
 */
void KDTree::
clear() {
  _nodes.clear();
  _leaves.clear();
  _inputs.clear();
}

/**
 * Adds an input object to the tree.  When the tree is built, the set of
 * objects are partitioned.
 */
void KDTree::
add_input(const LPoint3 &min, const LPoint3 &max, int value) {
  Input input;
  input.mins = min;
  input.maxs = max;
  input.value = value;
  _inputs.push_back(std::move(input));
}

/**
 * Creates a subtree from the indicated set of objects, partitioning as
 * necessary into further subtrees.  Returns the index of the root node
 * of the created subtree.  If the subtree root is a single leaf, the returned
 * index is negative, and the actual index into the leaf array is ~index.
 */
int KDTree::
make_subtree(const vector_int &objects) {
  if (objects.size() == 0) {
    // If the subtree has no objects, create an empty leaf.
    return ~make_leaf(-1);

  } else if (objects.size() == 1) {
    // We have just one object, so put it in a leaf.
    return ~make_leaf(_inputs[objects[0]].value);
  }

  // We have some objects.  Determine whether to put them in a single
  // leaf or partition them into further subtrees.  If we have a single object
  // or all the objects have the same value (cluster index), then we can
  // collapse all of them into a single leaf node.

  bool all_same = true;
  int first_object_value = _inputs[objects[0]].value;
  for (size_t i = 1; i < objects.size(); i++) {
    if (_inputs[i].value != first_object_value) {
      // Object values (cluster indices) differ.  They must be partitioned.
      all_same = false;
      break;
    }
  }

  if (all_same) {
    // All of the objects have the same value/cluster index.  We can collapse
    // them into a single leaf.
    return ~make_leaf(_inputs[objects[0]].value);
  }

  // We have to partition the objects into two half-spaces.

  // There are 6 different splits to choose from:
  // X-min, Y-min, Z-min, X-max, Y-max, or Z-max.
  pvector<SplitCandidate> splits = {
    SplitCandidate(0, false),
    SplitCandidate(1, false),
    SplitCandidate(2, false),
    SplitCandidate(0, true),
    SplitCandidate(1, true),
    SplitCandidate(2, true)
  };
  int best_split = pick_best_split(splits, objects);
  SplitCandidate &split = splits[best_split];

  int index = make_node(split.axis, split.dist);
  split_and_make_subtrees(index, split.left, split.right, split.split);

  return index;
}

/**
 * Returns the value associated with the leaf node that is closest to the
 * indicated point in space.  This is a nearest neighbor search.
 */
int KDTree::
get_nearest_leaf_value_from_point(const LPoint3 &point, int node) const {
  if (node == INVALID_NODE) {
    return -1;
  }

  int next_branch = INVALID_NODE;
  int other_branch = INVALID_NODE;

  //if (point[])
  return -1;
}

/**
 * Returns the value associated with the leaf node that contains the indicated
 * point in space.
 */
int KDTree::
get_leaf_value_from_point(const LPoint3 &point, int head_node) const {
  nassertr(head_node >= 0 && head_node < (int)_nodes.size(), -1);

  int i = head_node;
  while (i >= 0) {
    const Node *node = &_nodes[i];
    if (point[node->axis] >= node->dist) {
      i = node->right_child;

    } else {
      i = node->left_child;
    }
  }

  return _leaves[~i].value;
}

/**
 * Returns the unique set of leaf values for leaves that intersect the
 * indicated bounding volume.  Does not report empty leaves with values of
 * -1.
 */
void KDTree::
get_leaf_values_containing_volume(const GeometricBoundingVolume *volume,
                                  BitArray &values, int head_node) const {
  std::stack<int> stack;
  stack.push(head_node);

  while (!stack.empty()) {
    int node_index = stack.top();
    stack.pop();

    if (node_index >= 0) {
      const Node *node = &_nodes[node_index];

      // Construct a BoundingPlane for the node's splitting plane.  This is
      // unfortunate, but it makes it convenient to test against
      // BoundingVolumes of any type.
      int flags = node->plane.contains(volume);

      if (flags == BoundingVolume::IF_no_intersection) {
        // Completely in front of the plane.  Traverse right.
        stack.push(node->right_child);

      } else if ((flags & BoundingVolume::IF_all) != 0) {
        // Completely behind the plane.  Traverse left.
        stack.push(node->left_child);

      } else {
        // The volume spans the plane.  Traverse both directions.
        stack.push(node->right_child);
        stack.push(node->left_child);
      }

    } else {
      const Leaf *leaf = &_leaves[~node_index];
      if (leaf->value != -1) {
        values.set_bit(leaf->value);
      }
    }
  }
}

/**
 * Returns true if the indicated volume is contained within a leaf that
 * has a value from the indicated set.
 */
bool KDTree::
is_volume_in_leaf_set(const GeometricBoundingVolume *vol, const BitArray &set,
                      int head_node) const {
  std::stack<int> stack;
  stack.push(head_node);

  TypeHandle vol_type_handle = vol->get_type();

  while (!stack.empty()) {
    int node_index = stack.top();
    stack.pop();

    if (node_index >= 0) {
      const Node *node = &_nodes[node_index];

      int flags;
      if (vol_type_handle == BoundingSphere::get_class_type()) {
        flags = node->plane.contains_sphere((const BoundingSphere *)vol);
      } else if (vol_type_handle == BoundingBox::get_class_type()) {
        flags = node->plane.contains_box((const BoundingBox *)vol);
      } else if (vol_type_handle == BoundingHexahedron::get_class_type()) {
        flags = node->plane.contains_hexahedron((const BoundingHexahedron *)vol);
      } else if (vol_type_handle == BoundingPlane::get_class_type()) {
        flags = node->plane.contains_plane((const BoundingPlane *)vol);
      } else if (vol_type_handle == BoundingLine::get_class_type()) {
        flags = node->plane.contains_line((const BoundingLine *)vol);
      } else {
        assert(false);
      }

      if (flags == BoundingVolume::IF_no_intersection) {
        // Completely in front of the plane.  Traverse right.
        stack.push(node->right_child);

      } else if ((flags & BoundingVolume::IF_all) != 0) {
        // Completely behind the plane.  Traverse left.
        stack.push(node->left_child);

      } else {
        // The volume spans the plane.  Traverse both directions.
        stack.push(node->right_child);
        stack.push(node->left_child);
      }

    } else {
      const Leaf *leaf = &_leaves[~node_index];
      if (leaf->value != -1 && set.get_bit(leaf->value)) {
        return true;
      }
    }
  }

  return false;
}

/**
 *
 */
void KDTree::
split_and_make_subtrees(int parent, vector_int &left, vector_int &right,
                        vector_int &split) {
  // Clip the objects that need splitting.
  for (size_t i = 0; i < split.size(); i++) {
    Input split_orig = _inputs[split[i]];

    // The left split takes the place of the original object.
    Input &left_split = _inputs[split[i]];
    left_split.mins = split_orig.mins;
    left_split.maxs = split_orig.maxs;
    // Clip the left side maximum to the split position.
    left_split.maxs[_nodes[parent].axis] = _nodes[parent].dist;
    left_split.value = split_orig.value;
    int left_index = split[i];
    left.push_back(left_index);

    Input right_split;
    right_split.mins = split_orig.mins;
    // Clip the right side minimum to the split position.
    right_split.mins[_nodes[parent].axis] = _nodes[parent].dist;
    right_split.maxs = split_orig.maxs;
    right_split.value = split_orig.value;
    int right_index = (int)_inputs.size();
    _inputs.push_back(std::move(right_split));
    right.push_back(right_index);
  }

  // Now create the subtrees for each side.
  _nodes[parent].left_child = make_subtree(left);
  _nodes[parent].right_child = make_subtree(right);
}

/**
 *
 */
int KDTree::
make_leaf(int value) {
  int index = (int)_leaves.size();
  _leaves.push_back(Leaf());
  _leaves[index].value = value;
  return index;
}

/**
 *
 */
int KDTree::
make_node(unsigned char axis, PN_stdfloat dist) {
  int index = (int)_nodes.size();
  _nodes.push_back(Node());
  LPlane plane(0, 0, 0, -dist);
  plane[axis] = 1;
  _nodes[index].plane = BoundingPlane(plane);
  _nodes[index].plane.local_object();
  _nodes[index].axis = axis;
  _nodes[index].dist = dist;
  _nodes[index].left_child = INVALID_NODE;
  _nodes[index].right_child = INVALID_NODE;
  return index;
}

/**
 * Picks the best split out of all specified candidates.
 */
int KDTree::
pick_best_split(pvector<SplitCandidate> &splits, const vector_int &objects) {
  vector_int candidates;
  for (size_t i = 0; i < splits.size(); i++) {
    candidates.push_back(i);
    SplitCandidate &split = splits[i];
    // Perform the partition for each split.
    partition_along_axis(split.axis, split.dist, objects, split.left,
                         split.right, split.split, split.min_point);
  }

  // Now eliminate candidates down to the best choice(s).

  for (auto it = candidates.begin(); it != candidates.end();) {
    int index = *it;
    if (splits[index].left.empty() || splits[index].right.empty()) {
      // Eliminate one-sided candidates.
      it = candidates.erase(it);
    } else {
      ++it;
    }
  }
  assert(!candidates.empty());

  if (candidates.size() == 1) {
    return candidates[0];
  }

  int lowest_split_count = INT_MAX;
  for (size_t i = 0; i < candidates.size(); i++) {
    int index = candidates[i];
    if (splits[index].split.size() < lowest_split_count) {
      lowest_split_count = splits[index].split.size();
    }
  }

  // Eliminate candidates that don't have the lowest split count.
  for (auto it = candidates.begin(); it != candidates.end();) {
    int index = *it;
    if (splits[index].split.size() != lowest_split_count) {
      it = candidates.erase(it);
    } else {
      it++;
    }
  }
  assert(!candidates.empty());

  if (candidates.size() == 1) {
    return candidates[0];
  }

  int lowest_delta = INT_MAX;
  for (size_t i = 0; i < candidates.size(); i++) {
    int index = candidates[i];
    int delta = std::abs((int)splits[index].right.size() - (int)splits[index].left.size());
    if (delta < lowest_delta) {
      lowest_delta = delta;
    }
  }

  // Eliminate candidates that don't have the lowest delta.
  for (auto it = candidates.begin(); it != candidates.end();) {
    int index = *it;
    int delta = std::abs((int)splits[index].right.size() - (int)splits[index].left.size());
    if (delta != lowest_delta) {
      it = candidates.erase(it);
    } else {
      it++;
    }
  }
  assert(!candidates.empty());

  // We now have the best split(s) to choose from.  If there are still
  // multiple candidates, any one will work just as well, so just pick
  // the first one.
  return candidates[0];
}

/**
 *
 */
void KDTree::
partition_along_axis(unsigned char split_axis, PN_stdfloat &split_pos, const vector_int &objects,
                     vector_int &left, vector_int &right,
                     vector_int &needs_splitting, bool use_min) {
  auto compare = [](const PN_stdfloat &a, const PN_stdfloat &b) -> bool {
    return a < b;
  };

  // Get a unique set of distances along the split axis.
  vector_stdfloat axis_dists;
  for (size_t i = 0; i < objects.size(); i++) {
    PN_stdfloat dist = use_min ?
      _inputs[objects[i]].mins[split_axis] :
      _inputs[objects[i]].maxs[split_axis];
    if (std::find(axis_dists.begin(), axis_dists.end(), dist) == axis_dists.end()) {
      axis_dists.push_back(dist);
    }
  }

  // Now get the median split distance.

  //if (axis_dists.size() % 2 == 0) {
  //  auto it1 = axis_dists.begin() + axis_dists.size() / 2 - 1;
  //  auto it2 = axis_dists.begin() + axis_dists.size() / 2;
  //  std::nth_element(axis_dists.begin(), it1, axis_dists.end(), compare);
  //  std::nth_element(axis_dists.begin(), it2, axis_dists.end(), compare);
  //  split_pos = ((*it1) + (*it2)) / 2;

  //} else {
  if (use_min) {
    auto it = axis_dists.begin() + (axis_dists.size() / 2);
    std::nth_element(axis_dists.begin(), it, axis_dists.end(), compare);
    split_pos = *it;

  } else {
    auto it = axis_dists.begin() + (axis_dists.size() / 2) - 1;
    std::nth_element(axis_dists.begin(), it, axis_dists.end(), compare);
    split_pos = *it;
  }

  //}

  // Partition objects to left or right of the split plane.
  for (size_t i = 0; i < objects.size(); i++) {
    const Input *obj = &_inputs[objects[i]];

    if (obj->maxs[split_axis] > split_pos && obj->mins[split_axis] >= split_pos) {
      // Entirely on the right (in front) of the plane.
      right.push_back(objects[i]);

    } else if (obj->maxs[split_axis] <= split_pos && obj->mins[split_axis] < split_pos) {
      // Entirely on the left (behind) of the plane.
      left.push_back(objects[i]);

    } else {
      // Object crosses the split plane.  Need to split it.
      needs_splitting.push_back(objects[i]);
    }
  }
}

/**
 * Returns the approximate number of bytes the tree takes up in memory.
 */
size_t KDTree::
get_memory_size() const {
  return (sizeof(Node) * _nodes.size()) + (sizeof(Leaf) * _leaves.size());
}

/**
 * Writes the nodes and leaves of the tree to the indicated datagram.
 */
void KDTree::
write_datagram(Datagram &dg) const {
  dg.add_uint32(_nodes.size());
  for (size_t i = 0; i < _nodes.size(); i++) {
    dg.add_int32(_nodes[i].left_child);
    dg.add_int32(_nodes[i].right_child);
    dg.add_stdfloat(_nodes[i].dist);
    dg.add_uint8(_nodes[i].axis);
  }

  dg.add_uint32(_leaves.size());
  for (size_t i = 0; i < _leaves.size(); i++) {
    dg.add_int32(_leaves[i].value);
  }
}

/**
 * Reads in the nodes and leaves of the tree from the indicated datagram.
 */
void KDTree::
read_datagram(DatagramIterator &scan) {
  _nodes.resize(scan.get_uint32());
  for (size_t i = 0; i < _nodes.size(); i++) {
    _nodes[i].left_child = scan.get_int32();
    _nodes[i].right_child = scan.get_int32();
    _nodes[i].dist = scan.get_stdfloat();
    _nodes[i].axis = scan.get_uint8();
    LPlane plane(0, 0, 0, -_nodes[i].dist);
    plane[_nodes[i].axis] = 1;
    _nodes[i].plane = BoundingPlane(plane);
    _nodes[i].plane.local_object();
  }

  _leaves.resize(scan.get_uint32());
  for (size_t i = 0; i < _leaves.size(); i++) {
    _leaves[i].value = scan.get_int32();
  }
}

/**
 * Recursive implementation of output().
 */
void KDTree::
r_output(int node_index, std::ostream &out, int indent_level) const {
  if (node_index < 0) {
    const Leaf *leaf = &_leaves[~node_index];
    indent(out, indent_level) << "leaf: value " << leaf->value << "\n";

  } else {
    const Node *node = &_nodes[node_index];
    indent(out, indent_level) << "node: axis " << node->axis << " dist " << node->dist << "\n";
    r_output(node->left_child, out, indent_level + 2);
    r_output(node->right_child, out, indent_level + 2);
  }
}
