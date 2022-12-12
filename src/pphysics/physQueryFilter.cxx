/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physQueryFilter.cxx
 * @author brian
 * @date 2021-05-22
 */

#include "physQueryFilter.h"
#include "physRigidActorNode.h"
#include "config_pphysics.h"
#include "physx_shaders.h"

IMPLEMENT_CLASS(PhysQueryFilterCallbackData);

/**
 *
 */
PhysBaseQueryFilter::
PhysBaseQueryFilter(CallbackObject *filter_callback) :
  _filter_callback(filter_callback)
{
}

physx::PxQueryHitType::Enum PhysBaseQueryFilter::
preFilter(const physx::PxFilterData &filter_data, const physx::PxShape *shape,
          const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) {

  physx::PxFilterData shape_data = shape->getQueryFilterData();

  // Query FilterData
  // word0: (block mask | touch mask)
  // word1: block/solid mask
  // word2: touch mask
  // word3: collision group
  ////////////////////////////////////
  // Shape FilterData
  // word0: contents mask
  // word1: collision group

  // word1 is the block mask, word2 is the touch mask.

  if (pphysics_cat.is_debug()) {
    pphysics_cat.debug()
      << "Prefilter\n";
    pphysics_cat.debug()
      << "block mask: " << BitMask32(filter_data.word1)
      << "\ntouch mask: " << BitMask32(filter_data.word2)
      << "\nshape mask: " << BitMask32(shape_data.word0)
      << "\n";
  }

  physx::PxQueryHitType::Enum ret;

  if ((filter_data.word1 & shape_data.word0) != 0) {
    // Blocking intersection.
    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Blocking\n";
    }
    ret = physx::PxQueryHitType::eBLOCK;

  } else if ((filter_data.word2 & shape_data.word0) != 0) {
    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Touching\n";
    }
    // Touching/passthrough intersection.
    ret = physx::PxQueryHitType::eTOUCH;

  } else {
    // Nothing.
    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Nothing\n";
    }
    return physx::PxQueryHitType::eNONE;
  }

  // If we got here, we passed our built-in tests.
  // If there's a user-provided callback filter, invoke that.
  if (_filter_callback != nullptr && actor->userData != nullptr) {
    PhysQueryFilterCallbackData cbdata;
    cbdata._actor = (PhysRigidActorNode *)actor->userData;
    cbdata._shape = (PhysShape *)shape->userData;
    cbdata._shape_from_collide_mask = shape_data.word0;
    cbdata._into_collide_mask = filter_data.word1;
    cbdata._result = (int)ret;
    _filter_callback->do_callback(&cbdata);
    ret = (physx::PxQueryHitType::Enum)cbdata.get_result();
  }

  return ret;
}

physx::PxQueryHitType::Enum PhysBaseQueryFilter::
postFilter(const physx::PxFilterData &filter_data, const physx::PxQueryHit &hit) {
  return physx::PxQueryHitType::eNONE;
}
