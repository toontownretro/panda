/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physQueryFilter.h
 * @author brian
 * @date 2021-05-22
 */

#ifndef PHYSQUERYFILTER_H
#define PHYSQUERYFILTER_H

#include "pandabase.h"
#include "physx_includes.h"
#include "nodePath.h"

/**
 * Base query filter that checks for common block or touch bits.
 */
class EXPCL_PANDA_PPHYSICS PhysBaseQueryFilter : public physx::PxQueryFilterCallback {
PUBLISHED:
  PhysBaseQueryFilter();

public:
  virtual physx::PxQueryHitType::Enum preFilter(
    const physx::PxFilterData &filter_data, const physx::PxShape *shape,
    const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) override;

  virtual physx::PxQueryHitType::Enum postFilter(
    const physx::PxFilterData &filter_data, const physx::PxQueryHit &hit) override;
};

/**
 * A query filter that either excludes or exclusively includes actor nodes who
 * are descendants of a particular parent node.  Also takes into account the
 * blocking and touching bitmasks.
 */
class EXPCL_PANDA_PPHYSICS PhysQueryNodeFilter : public PhysBaseQueryFilter {
PUBLISHED:
  enum FilterType {
    // Exclude actor nodes who are descendants of the parent node.
    FT_exclude,
    // Only include actor nodes who are descendants of the parent node.
    FT_exclusive_include,
  };

  PhysQueryNodeFilter(const NodePath &parent_node, FilterType filter_type);

  INLINE void set_parent(const NodePath &parent);
  INLINE NodePath get_parent() const;

  INLINE void set_filter_type(FilterType type);
  INLINE FilterType get_filter_type() const;

public:
  virtual physx::PxQueryHitType::Enum preFilter(
    const physx::PxFilterData &filter_data, const physx::PxShape *shape,
    const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) override;

private:
  NodePath _parent_node;

  // How to filter nodes that are descendants of the parent.
  FilterType _filter_type;
};

#include "physQueryFilter.I"

#endif // PHYSQUERYFILTER_H
