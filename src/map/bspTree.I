/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspTree.I
 * @author brian
 * @date 2021-12-24
 */

/**
 *
 */
INLINE const BSPTree::Node *BSPTree::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), nullptr);
  return &_nodes[n];
}

/**
 *
 */
INLINE const BSPTree::Leaf *BSPTree::
get_leaf(int n) const {
  nassertr(n >= 0 && n < (int)_leaves.size(), nullptr);
  return &_leaves[n];
}

/**
 * Returns the index of the parent of the node at index n.
 */
INLINE int BSPTree::
get_node_parent(int n) const {
  nassertr(n >= 0 && n < (int)_node_parents.size(), -1);
  return _node_parents[n];
}

/**
 * Returns the index of the parent of the leaf at index n.
 */
INLINE int BSPTree::
get_leaf_parent(int n) const {
  nassertr(n >= 0 && n < (int)_leaf_parents.size(), -1);
  return _leaf_parents[n];
}
