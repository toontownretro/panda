/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aabbTree.cxx
 * @author brian
 * @date 2021-07-14
 */

#include "aabbTree.h"

/**
 *
 */
void AABBTreeInt::
write_datagram(Datagram &dg) const {
  AABBTree<int>::write_datagram(dg);
  for (size_t i = 0; i < _nodes.size(); i++) {
    const Node *node = &_nodes[i];
    if (node->is_leaf()) {
      dg.add_int32(node->value);
    }
  }
}

/**
 *
 */
void AABBTreeInt::
read_datagram(DatagramIterator &scan) {
  AABBTree<int>::read_datagram(scan);
  for (size_t i = 0; i < _nodes.size(); i++) {
    Node *node = &_nodes[i];
    if (node->is_leaf()) {
      node->value = scan.get_int32();
    }
  }
}
