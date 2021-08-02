/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file kdTree.I
 * @author brian
 * @date 2021-07-24
 */

/**
 * Returns the number of nodes in the tree.
 */
INLINE size_t KDTree::
get_num_nodes() const {
  return _nodes.size();
}

/**
 * Returns the nth node of the tree.
 */
INLINE const KDTree::Node *KDTree::
get_node(size_t n) const {
  nassertr(n < _nodes.size(), nullptr);
  return &_nodes[n];
}

/**
 * Returns the number of leaves in the tree.
 */
INLINE size_t KDTree::
get_num_leaves() const {
  return _leaves.size();
}

/**
 * Returns the nth leaf of the tree.
 */
INLINE const KDTree::Leaf *KDTree::
get_leaf(size_t n) const {
  nassertr(n < _leaves.size(), nullptr);
  return &_leaves[n];
}

/**
 * Outputs a text representation of the tree to the indicated output stream.
 */
INLINE void KDTree::
output(std::ostream &out) const {
  r_output(0, out, 0);
}
