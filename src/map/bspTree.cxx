/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspTree.cxx
 * @author brian
 * @date 2021-12-24
 */

#include "bspTree.h"
#include <stack>
#include "bamReader.h"

IMPLEMENT_CLASS(BSPTree);

/**
 * Returns the index of the empty leaf that the point resides in, or -1
 * if the point is in a solid leaf.
 */
int BSPTree::
get_leaf_value_from_point(const LPoint3 &point, int head_node) const {
  int node_id = head_node;
  while (node_id >= 0) {
    const Node *node = &_nodes[node_id];
    PN_stdfloat dist = node->plane.dist_to_plane(point);
    if (dist >= 0.0f) {
      node_id = node->children[FRONT_CHILD];
    } else {
      node_id = node->children[BACK_CHILD];
    }
  }

  const Leaf *leaf = &_leaves[~node_id];
  if (leaf->solid) {
    return -1;
  } else {
    return leaf->value;
  }
}

/**
 *
 */
void BSPTree::
get_leaf_values_containing_box(const LPoint3 &mins, const LPoint3 &maxs, ov_set<int> &values) const {
  std::stack<int> node_stack;
  node_stack.push(0);

  LPoint3 c = (maxs + mins) * 0.5f;
  LPoint3 e = maxs - c;

  while (!node_stack.empty()) {
    int node_id = node_stack.top();
    node_stack.pop();

    if (node_id >= 0) {
      const Node *node = &_nodes[node_id];

      // Projection interval radius.
      PN_stdfloat r = e[0] * cabs(node->plane[0]) + e[1] * cabs(node->plane[1]) + e[2] * cabs(node->plane[2]);
      PN_stdfloat d = node->plane.dist_to_plane(c);

      if (d <= -r) {
        // Completely behind plane, traverse back.
        node_stack.push(node->children[BACK_CHILD]);

      } else if (d <= r) {
        // Spans plane.
        node_stack.push(node->children[FRONT_CHILD]);
        node_stack.push(node->children[BACK_CHILD]);

      } else {
        // Completely in front of plane, traverse forward.
        node_stack.push(node->children[FRONT_CHILD]);
      }

    } else {
      // We reached a leaf node.
      const Leaf *leaf = &_leaves[~node_id];
      if (!leaf->solid && leaf->value != -1) {
        values.push_back(leaf->value);
      }
    }
  }

  values.sort();
}

/**
 *
 */
void BSPTree::
get_leaf_values_containing_sphere(const LPoint3 &origin, PN_stdfloat radius, ov_set<int> &values) const {
  std::stack<int> node_stack;
  node_stack.push(0);

  while (!node_stack.empty()) {
    int node_id = node_stack.top();
    node_stack.pop();

    if (node_id >= 0) {
      const Node *node = &_nodes[node_id];

      PN_stdfloat dist = node->plane.dist_to_plane(origin);

      if (dist <= -radius) {
        // Completely behind plane, traverse back.
        node_stack.push(node->children[BACK_CHILD]);

      } else if (dist <= radius) {
        // Spans plane.
        node_stack.push(node->children[FRONT_CHILD]);
        node_stack.push(node->children[BACK_CHILD]);

      } else {
        // Completely in front of plane, traverse forward.
        node_stack.push(node->children[FRONT_CHILD]);
      }

    } else {
      // We reached a leaf node.
      const Leaf *leaf = &_leaves[~node_id];
      if (!leaf->solid && leaf->value != -1) {
        values.push_back(leaf->value);
      }
    }
  }

  values.sort();
}

/**
 *
 */
void BSPTree::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void BSPTree::
write_datagram(BamWriter *manager, Datagram &me) {
  SpatialPartition::write_datagram(manager, me);

  me.add_uint32(_nodes.size());
  for (size_t i = 0; i < _nodes.size(); ++i) {
    me.add_int32(_nodes[i].children[0]);
    me.add_int32(_nodes[i].children[1]);
    _nodes[i].plane.write_datagram(me);
  }

  me.add_uint32(_leaves.size());
  for (size_t i = 0; i < _leaves.size(); ++i) {
    me.add_int32(_leaves[i].value);
    me.add_bool(_leaves[i].solid);
  }
}

/**
 *
 */
void BSPTree::
fillin(DatagramIterator &scan, BamReader *manager) {
  SpatialPartition::fillin(scan, manager);

  _nodes.resize(scan.get_uint32());
  for (size_t i = 0; i < _nodes.size(); ++i) {
    _nodes[i].children[0] = scan.get_int32();
    _nodes[i].children[1] = scan.get_int32();
    _nodes[i].plane.read_datagram(scan);
  }

  _leaves.resize(scan.get_uint32());
  for (size_t i = 0; i < _leaves.size(); ++i) {
    _leaves[i].value = scan.get_int32();
    _leaves[i].solid = scan.get_bool();
  }
}

/**
 *
 */
TypedWritable *BSPTree::
make_from_bam(const FactoryParams &params) {
  BSPTree *tree = new BSPTree;

  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  tree->fillin(scan, manager);

  return tree;
}
