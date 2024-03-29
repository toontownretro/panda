/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelIndex.I
 * @author brian
 * @date 2021-03-04
 */

/**
 * Returns the number of indexed trees.
 */
INLINE int ModelIndex::
get_num_trees() const {
  return (int)_trees.size();
}

/**
 * Returns the nth indexed tree.
 */
INLINE ModelIndex::Tree *ModelIndex::
get_tree(int n) const {
  return _trees[n];
}
