/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspTree.h
 * @author brian
 * @date 2021-12-24
 */

#ifndef BSPTREE_H
#define BSPTREE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "plane.h"
#include "vector_int.h"
#include "pvector.h"
#include "spatialPartition.h"

#define BACK_CHILD 0
#define FRONT_CHILD 1

class FactoryParams;
class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;

/**
 * A binary tree that partitions the world into disjoint convex sub-spaces.
 * This is similar to a K-D tree except the splitting planes are arbitrary--
 * they do not have to be axis aligned.  A BSP tree is generally built up from
 * a set of polygons where splitting planes correspond to polygon planes.
 *
 * The purpose of this class is to store the BSP tree nodes and leaves, and
 * provide methods to query the tree.  It does not have any logic for actually
 * building the tree, which is done in VisBuilderBSP.
 *
 * In the context of the visibility system, leaf nodes correspond to visibility
 * cells.
 */
class EXPCL_PANDA_MAP BSPTree : public SpatialPartition {
  DECLARE_CLASS(BSPTree, SpatialPartition);

PUBLISHED:
  /**
   * Data for a single node of the tree.
   */
  class Node {
  public:
    INLINE Node() : children{ 0, 0 } { }

    // Children indices, behind and in front of the node's splitting plane.
    // 0 indicates no child, > 0 is another node, < 0 is a leaf node.
    // If it's a leaf node, ~child converts it to an index into the leaf node
    // array.  For regular nodes, index into the node array is child-1.
    int children[2];

    // The node's splitting plane.  It is completely arbitrary.
    LPlane plane;

  PUBLISHED:
    /**
     * Returns the index of the child behind this node's splitting plane.
     */
    INLINE int get_back_child() const { return children[BACK_CHILD]; }
    /**
     * Returns the index of the child in front of this node's splitting plane.
     */
    INLINE int get_front_child() const { return children[FRONT_CHILD]; }
    /**
     * Returns the node's splitting plane.
     */
    INLINE const LPlane &get_plane() const { return plane; }
  };

  class Leaf {
  public:
    INLINE Leaf() : solid(false), value(-1) { }

    int value;
    bool solid;

  PUBLISHED:
    INLINE bool is_solid() const { return solid; }
    INLINE int get_value() const { return value; }
  };

  virtual int get_leaf_value_from_point(const LPoint3 &point, int head_node = 0) const override;
  virtual void get_leaf_values_containing_box(const LPoint3 &mins, const LPoint3 &maxs, ov_set<int> &values) const override;
  virtual void get_leaf_values_containing_sphere(const LPoint3 &center, PN_stdfloat radius, ov_set<int> &values) const override;

public:
  typedef pvector<Node> Nodes;
  Nodes _nodes;

  typedef pvector<Leaf> Leaves;
  Leaves _leaves;

  vector_int _leaf_parents;
  vector_int _node_parents;

public:
  static void register_with_read_factory();

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;

private:
  static TypedWritable *make_from_bam(const FactoryParams &params);
};

#include "bspTree.I"

#endif // BSPTREE_H
