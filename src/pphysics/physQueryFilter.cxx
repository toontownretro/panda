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

/**
 *
 */
PhysBaseQueryFilter::
PhysBaseQueryFilter()
{
}

physx::PxQueryHitType::Enum PhysBaseQueryFilter::
preFilter(const physx::PxFilterData &filter_data, const physx::PxShape *shape,
          const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) {

  // word1 is the block mask, word2 is the touch mask.

  physx::PxFilterData shape_data = shape->getQueryFilterData();

  if (pphysics_cat.is_debug()) {
    pphysics_cat.debug()
      << "Prefilter\n";
    pphysics_cat.debug()
      << "block mask: " << BitMask32(filter_data.word1)
      << "\ntouch mask: " << BitMask32(filter_data.word2)
      << "\nshape mask: " << BitMask32(shape_data.word0)
      << "\n";
  }

  if ((filter_data.word1 & shape_data.word0) != 0) {
    // Blocking intersection.
    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Blocking\n";
    }
    return physx::PxQueryHitType::eBLOCK;

  } else if ((filter_data.word2 & shape_data.word0) != 0) {
    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Touching\n";
    }
    // Touching/passthrough intersection.
    return physx::PxQueryHitType::eTOUCH;

  } else {
    // Nothing.
    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Nothing\n";
    }
    return physx::PxQueryHitType::eNONE;
  }
}

physx::PxQueryHitType::Enum PhysBaseQueryFilter::
postFilter(const physx::PxFilterData &filter_data, const physx::PxQueryHit &hit) {
  return physx::PxQueryHitType::eNONE;
}

/**
 *
 */
PhysQueryNodeFilter::
PhysQueryNodeFilter(const NodePath &parent_node, FilterType filter_type) :
  _filter_type(filter_type),
  _parent_node(parent_node)
{
}

physx::PxQueryHitType::Enum PhysQueryNodeFilter::
preFilter(const physx::PxFilterData &filter_data, const physx::PxShape *shape,
          const physx::PxRigidActor *actor, physx::PxHitFlags &query_flags) {

  // Let the base filter determine the hit type.
  physx::PxQueryHitType::Enum hit_type = PhysBaseQueryFilter::
    preFilter(filter_data, shape, actor, query_flags);

  if (hit_type == physx::PxQueryHitType::eNONE) {
    // Base filter didn't pass, so we don't have to go any further.
    return hit_type;
  }

  if (actor->userData != nullptr) {
    PhysRigidActorNode *node = (PhysRigidActorNode *)actor->userData;
    NodePath np(node);
    if (_parent_node.is_ancestor_of(np)) {
      if (_filter_type == FT_exclude) {
        return physx::PxQueryHitType::eNONE;
      }
    } else if (_filter_type == FT_exclusive_include) {
      // We only want descendents of the parent node, and this node is not a
      // descendant, so exclude it.
      return physx::PxQueryHitType::eNONE;
    }

  } else if (_filter_type == FT_exclusive_include) {
    // If we are only including descendants and this actor was not associated
    // with a PandaNode, it gets filtered out.
    return physx::PxQueryHitType::eNONE;
  }

  return hit_type;
}
