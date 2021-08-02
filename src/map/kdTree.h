/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file kdTree.h
 * @author brian
 * @date 2021-07-24
 */

#ifndef KDTREE_H
#define KDTREE_H

#include "pandabase.h"
#include "numeric_types.h"
#include "pvector.h"
#include "luse.h"
#include "vector_int.h"
#include "pset.h"
#include "datagram.h"
#include "datagramIterator.h"

class GeometricBoundingVolume;

/**
 * A k-dimensional (k-d) tree is an axis-aligned binary space partitioning
 * (BSP) tree.  In this case, k is 3.  The universe is recursively partitioned
 * into half-spaces using axis-aligned hyperplanes.  The axis of the
 * partitioning hyperplane is determined by the depth of the node that is
 * being partitioned.  Leaf nodes in the tree represent disjointed axis-aligned
 * regions of the universe.  For the visibility system, non-empty leaf nodes
 * correspond to area clusters.
 */
class EXPCL_PANDA_MAP KDTree {
PUBLISHED:
#pragma pack(push, 1)
  class Node {
  PUBLISHED:
    // < 0 is a leaf node, ~child is leaf index.
    int left_child; // Child behind the hyperplane.
    int right_child; // Child on or in front of the hyperplane.

    // Node's partitioning hyperplane.
    PN_stdfloat dist;
    unsigned char axis;
  };

  class Leaf {
  PUBLISHED:
    int value;
  };
#pragma pack(pop)

  // Input objects for building the tree are hyperrectangles/AABBs.
  // These correspond to boxes of an area cluster.
  struct Input {
    LPoint3 mins, maxs;
    int value;
  };

  class SplitCandidate {
  public:
    INLINE SplitCandidate(unsigned char a, bool use_min) :
      axis(a), min_point(use_min) { }

    bool min_point;
    unsigned char axis;
    PN_stdfloat dist;
    vector_int left;
    vector_int right;
    vector_int split;
  };

  KDTree() = default;
  KDTree(const KDTree &copy) = default;
  KDTree(KDTree &&other);
  void operator = (KDTree &&other);

  void build();

  void clear();

  void add_input(const LPoint3 &mins, const LPoint3 &maxs, int value);

  int make_subtree(const vector_int &objects);

  int get_leaf_value_from_point(const LPoint3 &point, int head_node = 0) const;
  void get_leaf_values_containing_volume(const GeometricBoundingVolume *volume,
                                         pset<int> &values, int head_node = 0) const;

  size_t get_memory_size() const;

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

  INLINE size_t get_num_nodes() const;
  INLINE const Node *get_node(size_t n) const;

  INLINE size_t get_num_leaves() const;
  INLINE const Leaf *get_leaf(size_t n) const;

  INLINE void output(std::ostream &out) const;

private:
  void r_output(int node_index, std::ostream &out, int indent_level) const;

  int make_leaf(int value);
  int make_node(unsigned char axis, PN_stdfloat dist);

  void split_and_make_subtrees(int parent, vector_int &left, vector_int &right,
                               vector_int &split);

  void partition_along_axis(unsigned char axis, PN_stdfloat &dist, const vector_int &objects,
                            vector_int &left, vector_int &right,
                            vector_int &needs_splitting, bool use_min);

  int pick_best_split(pvector<SplitCandidate> &candidates, const vector_int &objects);

protected:
  typedef pvector<Node> Nodes;
  typedef pvector<Leaf> Leaves;
  typedef pvector<Input> Inputs;
  Nodes _nodes;
  Leaves _leaves;

  Inputs _inputs;
};

#include "kdTree.I"

#endif // KDTREE_H
