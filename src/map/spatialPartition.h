/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spatialPartition.h
 * @author brian
 * @date 2022-01-19
 */

#ifndef SPATIALPARTITION_H
#define SPATIALPARTITION_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "ordered_vector.h"
#include "luse.h"

/**
 * Abstract base class for a map's spatial partition.
 *
 * Partition used depends how the visibility information was baked.
 * The BSP-based method stores a binary space partition.
 * The voxel-based method stores a K-D tree.
 */
class EXPCL_PANDA_MAP SpatialPartition : public TypedWritableReferenceCount {
  DECLARE_CLASS(SpatialPartition, TypedWritableReferenceCount);

PUBLISHED:
  virtual int get_leaf_value_from_point(const LPoint3 &point, int head_node = 0) const=0;
  virtual void get_leaf_values_containing_box(const LPoint3 &mins, const LPoint3 &maxs, ov_set<int> &values) const=0;
  virtual void get_leaf_values_containing_sphere(const LPoint3 &center, PN_stdfloat radius, ov_set<int> &values) const=0;
};

#include "spatialPartition.I"

#endif // SPATIALPARTITION_H
