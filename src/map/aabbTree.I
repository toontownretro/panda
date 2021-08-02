/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aabbTree.I
 * @author brian
 * @date 2021-07-14
 */

/**
 *
 */
INLINE PN_stdfloat
aabb_hsurface_area(const LPoint3 &min, const LPoint3 &max) {
  PN_stdfloat width = max[0] - min[0];
  PN_stdfloat height = max[1] - min[1];
  PN_stdfloat depth = max[2] - min[2];

  return (width * height + width * depth + height * depth);
}

/**
 *
 */
INLINE PN_stdfloat
aabb_surface_area(const LPoint3 &min, const LPoint3 &max) {
  return 2.0f * aabb_hsurface_area(min, max);
}

/**
 *
 */
INLINE void
aabb_merge(const LPoint3 &min_a, const LPoint3 &max_a,
           const LPoint3 &min_b, const LPoint3 &max_b,
           LPoint3 &min, LPoint3 &max) {
  min[0] = std::min(min_a[0], min_b[0]);
  min[1] = std::min(min_a[1], min_b[1]);
  min[2] = std::min(min_a[2], min_b[2]);
  max[0] = std::max(max_a[0], max_b[0]);
  max[1] = std::max(max_a[1], max_b[1]);
  max[2] = std::max(max_a[2], max_b[2]);
}

/**
 *
 */
INLINE void
aabb_merge(const LPoint3 &min_a, const LPoint3 &max_a,
           const LPoint3 &point, LPoint3 &min, LPoint3 &max) {
  min[0] = std::min(point[0], min_a[0]);
  min[1] = std::min(point[1], min_a[1]);
  min[2] = std::min(point[2], min_a[2]);
  max[0] = std::max(point[0], max_a[0]);
  max[1] = std::max(point[1], max_a[1]);
  max[2] = std::max(point[2], max_a[2]);
}

/**
 *
 */
INLINE PN_stdfloat
aabb_delta(const LPoint3 &min, const LPoint3 &max, int axis) {
  return max[axis] - min[axis];
}

/**
 *
 */
INLINE int
aabb_major_axis(const LPoint3 &min, const LPoint3 &max) {
  LVector3 extents = max - min;
  if (extents[0] > extents[1] && extents[0] > extents[2]) {
    return 0;
  } else if (extents[1] > extents[0] && extents[1] > extents[2]) {
    return 1;
  } else {
    return 2;
  }
}

/**
 *
 */
template<typename T>
INLINE AABBTree<T>::
AABBTree()
{
}

/**
 * Adds the indicated leaf node to the tree.  When build() is called, all
 * leaves will be grouped into a hierarchy.
 */
template<typename T>
INLINE void AABBTree<T>::
add_leaf(const LPoint3 &min, const LPoint3 &max, const T &value) {
  size_t index = _prebuild_leaves.size();
  _prebuild_leaves.push_back(Node());
  _prebuild_leaves[index].min = min;
  _prebuild_leaves[index].max = max;
  _prebuild_leaves[index].value = value;
}

/**
 * Builds up the tree from the previously added leaves.
 */
template<typename T>
INLINE void AABBTree<T>::
build() {
  _nodes.clear();

  _nodes.reserve(_prebuild_leaves.size() * 2);

  // Start with the root node.
  _nodes.push_back(Node());
  vector_int leaves;
  leaves.reserve(_prebuild_leaves.size());
  for (int i = 0; i < (int)_prebuild_leaves.size(); i++) {
    leaves.push_back(i);
  }
  split_node(0, leaves, 0);

  _prebuild_leaves.clear();
}

/**
 *
 */
template<typename T>
INLINE void AABBTree<T>::
split_node(int n, const vector_int &leaves, int depth) {
  if (leaves.empty()) {
    return;
  }

  if (leaves.size() == 1) {
    // We found a home for this leaf.
    _nodes[n].min = _prebuild_leaves[leaves[0]].min;
    _nodes[n].max = _prebuild_leaves[leaves[0]].max;
    _nodes[n].value = _prebuild_leaves[leaves[0]].value;
    return;
  }

  // Enclose all of the leaves the node contains.
  _nodes[n].min.set(FLT_MAX, FLT_MAX, FLT_MAX);
  _nodes[n].max.set(FLT_MIN, FLT_MIN, FLT_MIN);
  for (int i = 0; i < (int)leaves.size(); i++) {
    const Node *leaf = &_prebuild_leaves[leaves[i]];
    aabb_merge(leaf->min, leaf->max,
               _nodes[n].min, _nodes[n].max,
               _nodes[n].min, _nodes[n].max);
  }

  int split_axis = depth % 3;

  vector_int left, right;
  PN_stdfloat split_pos = partition_leaves(split_axis, leaves, left, right, false);

  if (left.empty() || right.empty()) {
    left.clear();
    right.clear();
    // Re-partition using the min point instead.
    split_pos = partition_leaves(split_axis, leaves, left, right, true);

    //assert(!left.empty() && !right.empty());
  }

  //if (left.empty() || right.empty()) {
  //  if (left.empty()) {
  //    std::cout << "left is empty\n";
  //    std::cout << "Split axis " << split_axis << " dist " << split_pos << "\n";
  //    std::cout << "right:\n";
  //    for (size_t i = 0; i < right.size(); i++) {
  //      const Node *leaf = &_prebuild_leaves[right[i]];
  //      std::cout << "\t" << right[i] << " min " << leaf->min << " max " << leaf->max << " value " << leaf->value << "\n";
  //    }
  //  } else {
  //    std::cout << "right is empty\n";
  //    std::cout << "Split axis " << split_axis << " dist " << split_pos << "\n";
  //    std::cout << "left:\n";
  //    for (size_t i = 0; i < left.size(); i++) {
  //      const Node *leaf = &_prebuild_leaves[left[i]];
  //      std::cout << "\t" << left[i] << " min " << leaf->min << " max " << leaf->max << " value " << leaf->value << "\n";
  //    }
  //  }
  //}

  //Area 0 min -5680 -4736 -632 max -5168 -4224 -120
  //       min -5680 -4736 -120 max -5680 -4736 -120
  //
  //Area 1 min -5680 -4736 -120 max -5168 -4224 392
  //       min -5680 -4736 -120 max -5680 -4736 -120

  //20310  value 0
  // 20257  value 1

  // Create nodes for leaves behind and in front of the plane.
  int left_node = (int)_nodes.size();
  _nodes.push_back(Node());

  int right_node = (int)_nodes.size();
  _nodes.push_back(Node());

  _nodes[n].children[0] = left_node;
  _nodes[n].children[1] = right_node;

  split_node(left_node, left, depth + 1);
  split_node(right_node, right, depth + 1);
}

/**
 *
 */
template<typename T>
INLINE PN_stdfloat AABBTree<T>::
partition_leaves(int split_axis, const vector_int &leaves, vector_int &left, vector_int &right,
                 bool use_min) {
  auto compare = [](const PN_stdfloat &a, const PN_stdfloat &b) -> bool {
    return a < b;
  };

  // Get a unique set of distances along the split axis.
  vector_stdfloat axis_dists;
  for (size_t i = 0; i < leaves.size(); i++) {
    PN_stdfloat dist = use_min ?
      _prebuild_leaves[leaves[i]].min[split_axis] :
      _prebuild_leaves[leaves[i]].max[split_axis];
    if (std::find(axis_dists.begin(), axis_dists.end(), dist) == axis_dists.end()) {
      axis_dists.push_back(dist);
    }
  }

  // Now get the median split distance.

  PN_stdfloat split_pos;
  if (axis_dists.size() % 2 == 0) {
    auto it1 = axis_dists.begin() + axis_dists.size() / 2 - 1;
    auto it2 = axis_dists.begin() + axis_dists.size() / 2;
    std::nth_element(axis_dists.begin(), it1, axis_dists.end(), compare);
    std::nth_element(axis_dists.begin(), it2, axis_dists.end(), compare);
    split_pos = ((*it1) + (*it2)) / 2;

  } else {
    auto it = axis_dists.begin() + axis_dists.size() / 2;
    std::nth_element(axis_dists.begin(), it, axis_dists.end(), compare);
    split_pos = *it;
  }

  // Partition leaves to left or right of the split plane.
  for (size_t i = 0; i < leaves.size(); i++) {
    const Node *leaf = &_prebuild_leaves[leaves[i]];

    const LPoint3 &coord = use_min ?
      leaf->min : leaf->max;

    // FIXME: This might not be correct
    if (coord[split_axis] <= split_pos /*&& leaf->min[split_axis] <= split_pos*/) {
      // Entirely on the left.
      left.push_back(leaves[i]);

    } else if (coord[split_axis] > split_pos /*&& leaf->min[split_axis] > split_pos*/) {
      // Entirely on the right.
      right.push_back(leaves[i]);

    }/* else {
      // The leaf crosses the split axis.  Need to split the leaf and put on
      // both sides.

      Node left_node;
      left_node.min = leaf->min;
      left_node.max = leaf->max;
      left_node.max[split_axis] = split_pos;
      left_node.value = leaf->value;

      if (left_node.min[split_axis] == left_node.max[split_axis]) {
        right.push_back(leaves[i]);

      } else {
        Node right_node;
        right_node.min = leaf->min;
        right_node.min[split_axis] = split_pos;
        right_node.max = leaf->max;
        right_node.value = leaf->value;

        if (right_node.min[split_axis] == right_node.max[split_axis]) {
          left.push_back(leaves[i]);

        } else {
          int left_node_index = (int)_prebuild_leaves.size();
          _prebuild_leaves.push_back(std::move(left_node));
          int right_node_index = (int)_prebuild_leaves.size();
          _prebuild_leaves.push_back(std::move(right_node));

          left.push_back(left_node_index);
          right.push_back(right_node_index);
        }
      }
    }*/
  }

  return split_pos;
}

/**
 * Returns the index of the first leaf node that contains the indicated point,
 * or -1 if no leaf contains the point.
 */
template<typename T>
INLINE int AABBTree<T>::
get_leaf_containing_point(const LPoint3 &point, int head_node) const {
  if (_nodes.empty()) {
    return -1;
  }

  std::stack<int> stack;
  stack.push(head_node);

  while (!stack.empty()) {
    int node_index = stack.top();
    stack.pop();

    if (node_index == -1) {
      continue;
    }

    const Node *node = &_nodes[node_index];
    if (point[0] >= node->min[0] && point[0] <= node->max[0] &&
        point[1] >= node->min[1] && point[1] <= node->max[1] &&
        point[2] >= node->min[2] && point[2] <= node->max[2]) {

      if (node->is_leaf()) {
        return node_index;

      } else {
        stack.push(node->children[0]);
        stack.push(node->children[1]);
      }
    }
  }

  return -1;
}

/**
 * Returns the value associated with the indicated leaf node.
 */
template<typename T>
INLINE const T &AABBTree<T>::
get_leaf_value(int leaf) const {
  assert(leaf >= 0 && leaf < (int)_nodes.size());
  const Node *node = &_nodes[leaf];
  assert(node->is_leaf());
  return node->value;
}

/**
 * Determines the set of leaf nodes that overlap with the given bounding
 * volume.
 */
template<typename T>
INLINE void AABBTree<T>::
get_leaves_overlapping_volume(const GeometricBoundingVolume *volume,
                              vector_int &leaves, int head_node) const {
  if (_nodes.empty()) {
    return;
  }

  std::stack<int> stack;
  stack.push(head_node);

  while (!stack.empty()) {
    int node_index = stack.top();
    stack.pop();

    if (node_index == -1) {
      continue;
    }

    const Node *node = &_nodes[node_index];

    BoundingBox bbox(LPoint3(node->min[0], node->min[1], node->min[2]),
                     LPoint3(node->max[0], node->max[1], node->max[2]));
    bbox.local_object();

    if (bbox.contains(volume) != BoundingVolume::IF_no_intersection) {
      if (node->is_leaf()) {
        leaves.push_back(node_index);

      } else {
        // Visit the children.
        stack.push(node->children[0]);
        stack.push(node->children[1]);
      }
    }
  }
}

/**
 * Returns the nth node in the tree.
 */
template<typename T>
INLINE const typename AABBTree<T>::Node *AABBTree<T>::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), nullptr);
  return &_nodes[n];
}

/**
 * Returns the number of nodes in the tree.
 */
template<typename T>
INLINE int AABBTree<T>::
get_num_nodes() const {
  return (int)_nodes.size();
}

/**
 * Empties the entire tree.
 */
template<typename T>
INLINE void AABBTree<T>::
clear() {
  _nodes.clear();
}

/**
 *
 */
template<typename T>
INLINE void AABBTree<T>::
output(std::ostream &out) const {
  if (_nodes.empty()) {
    out << "empty";
  } else {
    r_output(&_nodes[0], out, 0);
  }
}

/**
 * Reserves memory for the indicated number of nodes.
 */
template<typename T>
INLINE void AABBTree<T>::
reserve(size_t node_count) {
  _nodes.reserve(node_count);
}

/**
 *
 */
template<typename T>
INLINE void AABBTree<T>::
r_output(const typename AABBTree<T>::Node *node, std::ostream &out, int indent_level) const {
  indent(out, indent_level);
  if (node->is_leaf()) {
    out << "leaf ";
  } else {
    out << "node ";
  }
  out << "min " << node->min << " max " << node->max;
  if (node->is_leaf()) {
    out << " value " << node->value;
  }
  out << "\n";
  if (!node->is_leaf()) {
    for (int i = 0; i < 2; i++) {
      r_output(&_nodes[node->children[i]], out, indent_level + 2);
    }
  }
}


/**
 * Writes the nodes of the tree to the indicated datagram.
 */
template<typename T>
INLINE void AABBTree<T>::
write_datagram(Datagram &dg) const {
  dg.add_int32(_nodes.size());
  for (size_t i = 0; i < _nodes.size(); i++) {
    const Node *node = &_nodes[i];
    if (node->is_leaf()) {
      dg.add_int32(node->children[0]);

    } else {
      dg.add_int32(node->children[0]);
      dg.add_int32(node->children[1]);
    }

    node->min.write_datagram(dg);
    node->max.write_datagram(dg);
  }
}

/**
 * Reads the nodes of the tree in from the indicated datagram.
 */
template<typename T>
INLINE void AABBTree<T>::
read_datagram(DatagramIterator &scan) {
  _nodes.resize(scan.get_int32());
  for (size_t i = 0; i < _nodes.size(); i++) {
    Node *node = &_nodes[i];
    node->children[0] = scan.get_int32();
    if (!node->is_leaf()) {
      node->children[1] = scan.get_int32();
    }
    node->min.read_datagram(scan);
    node->max.read_datagram(scan);
  }
}
