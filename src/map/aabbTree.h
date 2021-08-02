/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aabbTree.h
 * @author brian
 * @date 2021-07-14
 */

#ifndef AABBTREE_H
#define AABBTREE_H

#include "pandabase.h"
#include "luse.h"
#include "pvector.h"
#include "vector_int.h"
#include <stack>
#include <algorithm>
#include "geometricBoundingVolume.h"
#include "boundingBox.h"
#include "indent.h"
#include "config_map.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "vector_stdfloat.h"

/**
 * A binary axis-aligned bounding box tree.  Very similar to a K-D tree but
 * stores axis-aligned bounding boxes at the nodes and leaves instead of
 * splitting axes.  The bounding boxes of sibling nodes can potentially
 * overlap.
 */
template<typename T>
class AABBTree {
PUBLISHED:
  class Node {
  public:
    INLINE Node() : value() {
      children[0] = children[1] = -1;
    }
    int children[2];
    LPoint3 min, max;
    T value;

    INLINE bool is_leaf() const { return children[0] == -1; }
  };

  INLINE AABBTree();

  INLINE void add_leaf(const LPoint3 &min, const LPoint3 &max, const T &value);

  INLINE void build();

  INLINE int get_leaf_containing_point(const LPoint3 &point, int head_node = 0) const;

  INLINE const T &get_leaf_value(int leaf) const;

  INLINE const typename Node *get_node(int n) const;
  INLINE int get_num_nodes() const;

  INLINE void clear();

  INLINE void output(std::ostream &out) const;

  INLINE void reserve(size_t node_count);

  INLINE void write_datagram(Datagram &dg) const;
  INLINE void read_datagram(DatagramIterator &scan);

public:
  INLINE void get_leaves_overlapping_volume(const GeometricBoundingVolume *volume,
                                            vector_int &leaves, int head_node = 0) const;

private:
  INLINE void split_node(int index, const vector_int &leaves, int depth);

  INLINE void r_output(const typename Node *node, std::ostream &out, int indent_level) const;

  INLINE PN_stdfloat partition_leaves(int axis, const vector_int &leaves, vector_int &left,
                                      vector_int &right, bool use_min);

protected:
  typedef pvector<Node> Nodes;
  Nodes _nodes;

  // Initial flat list of leaves for doing a bottom-up tree build.
  Nodes _prebuild_leaves;
};

INLINE PN_stdfloat aabb_hsurface_area(const LPoint3 &min, const LPoint3 &max);
INLINE PN_stdfloat aabb_surface_area(const LPoint3 &min, const LPoint3 &max);
INLINE void aabb_merge(const LPoint3 &min_a, const LPoint3 &max_a, const LPoint3 &min_b,
                       const LPoint3 &max_b, LPoint3 &min, LPoint3 &max);
INLINE void aabb_merge(const LPoint3 &min_a, const LPoint3 &max_a, const LPoint3 &point,
                       LPoint3 &min, LPoint3 &max);
INLINE PN_stdfloat aabb_delta(const LPoint3 &min, const LPoint3 &max, int axis);
INLINE int aabb_major_axis(const LPoint3 &min, const LPoint3 &max);

/**
 * An AABB tree that stores integers at the leaves.
 */
class EXPCL_PANDA_MAP AABBTreeInt : public AABBTree<int> {
PUBLISHED:
  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);
};

#include "aabbTree.I"

#endif // AABBTREE_H
