/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animGraphNode.I
 * @author lachbr
 * @date 2021-02-18
 */

/**
 * Returns the number of the child/input linkings to this node.
 */
INLINE int AnimGraphNode::
get_num_children() const {
  return _children.size();
}

/**
 * Returns the nth child/input linking to this node.
 */
INLINE AnimGraphNode *AnimGraphNode::
get_child(int n) const {
  return _children[n];
}
