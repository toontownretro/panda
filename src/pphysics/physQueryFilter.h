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
#include "callbackData.h"
#include "callbackObject.h"
#include "pointerTo.h"

class PhysRigidActorNode;
class PhysShape;

/**
 * Base query filter that checks for common block or touch bits.
 */
class EXPCL_PANDA_PPHYSICS PhysBaseQueryFilter : public physx::PxQueryFilterCallback {
public:
  PhysBaseQueryFilter(CallbackObject *filter_callback = nullptr);

  virtual physx::PxQueryHitType::Enum preFilter(
    const physx::PxFilterData &filter_data, const physx::PxShape *shape,
    const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) override;

  virtual physx::PxQueryHitType::Enum postFilter(
    const physx::PxFilterData &filter_data, const physx::PxQueryHit &hit) override;

private:
  PT(CallbackObject) _filter_callback;
};

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysQueryFilterCallbackData : public CallbackData {
  DECLARE_CLASS(PhysQueryFilterCallbackData, CallbackData);

PUBLISHED:
  // Actor/shape we are considering testing intersection with.
  INLINE PhysRigidActorNode *get_actor() const;
  INLINE PhysShape *get_shape() const;
  INLINE unsigned int get_shape_contents_mask() const;
  INLINE unsigned int get_shape_collision_group() const;
  MAKE_PROPERTY(actor, get_actor);
  MAKE_PROPERTY(shape, get_shape);
  MAKE_PROPERTY(shape_contents_mask, get_shape_contents_mask);
  MAKE_PROPERTY(shape_collision_group, get_shape_collision_group);

  // Filtering properties of the query geometry.
  INLINE unsigned int get_solid_mask() const;
  INLINE unsigned int get_collision_group() const;
  MAKE_PROPERTY(solid_mask, get_solid_mask);
  MAKE_PROPERTY(collision_group, get_collision_group);

  // Filter callback should set the result to indicate if the filter
  // passes or fails.
  INLINE void set_result(int flag);
  INLINE int get_result() const;
  MAKE_PROPERTY(result, get_result, set_result);

private:
  PhysQueryFilterCallbackData() = default;

private:
  // Filtering properties of the geometry used for the query
  // (the ray, box, etc).
  unsigned int _solid_mask;
  unsigned int _collision_group;

  // The actor we are considering intersection with.
  PhysRigidActorNode *_actor;
  PhysShape *_shape;
  unsigned int _shape_contents_mask;
  unsigned int _shape_collision_group;

  // Holds the result of the filter callback.
  // False means to ignore the actor, true means to test for intersection
  // and report it.
  int _result;

  friend class PhysBaseQueryFilter;
};

#include "physQueryFilter.I"

#endif // PHYSQUERYFILTER_H
