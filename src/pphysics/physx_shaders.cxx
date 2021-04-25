/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physx_shaders.cxx
 * @author brian
 * @date 2021-04-18
 */

#include "physx_shaders.h"
#include "config_pphysics.h"
#include "physRigidActorNode.h"

#include "bitMask.h"

PandaSimulationFilterShader::CollisionGroupPair PandaSimulationFilterShader::_collision_table[32][32];

/**
 *
 */
physx::PxFilterFlags PandaSimulationFilterShader::
filter(physx::PxFilterObjectAttributes attributes0,
       physx::PxFilterData filter_data0,
       physx::PxFilterObjectAttributes attributes1,
       physx::PxFilterData filter_data1,
       physx::PxPairFlags &pair_flags,
       const void *constant_block,
       physx::PxU32 constant_block_size) {

  PX_UNUSED(constant_block);
  PX_UNUSED(constant_block_size);

  if (pphysics_cat.is_debug()) {
    pphysics_cat.debug()
      << "Running filter shader\n";
    pphysics_cat.debug()
      << "Mask0: " << BitMask32(filter_data0.word0) << "\n";
    pphysics_cat.debug()
      << "Mask1: " << BitMask32(filter_data1.word0) << "\n";
  }

  bool enable_collisions = false;

  if (filter_data0.word0 == 0 || filter_data1.word0 == 0) {
    // One or both of the shapes are not assigned to any collision groups.
    // Collisions will always occur.
    enable_collisions = true;

  } else {
    // Both shapes are assigned to one or more collision groups.  Check the
    // collision table to see if any groups have collisions enabled between
    // them.
    BitMask32 mask0 = filter_data0.word0;
    int group = mask0.get_lowest_on_bit();
    while (group >= 0) {
      BitMask32 mask1 = filter_data1.word0;
      int other_group = mask1.get_lowest_on_bit();
      while (other_group >= 0) {
        if (_collision_table[group][other_group]._enable_collisions) {
          enable_collisions = true;
          break;
        }

        mask1.clear_bit(other_group);
        other_group = mask1.get_lowest_on_bit();
      }

      if (enable_collisions) {
        break;
      }

      mask0.clear_bit(group);
      group = mask0.get_lowest_on_bit();
    }
  }

  if (pphysics_cat.is_debug()) {
    pphysics_cat.debug()
      << "Collide? " << enable_collisions << "\n";
  }

  if (!enable_collisions) {
    return physx::PxFilterFlag::eSUPPRESS;
  }

  // Handle triggers.
  if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1)) {
    pair_flags = physx::PxPairFlag::eTRIGGER_DEFAULT;
    return physx::PxFilterFlag::eDEFAULT;
  }

  pair_flags = physx::PxPairFlag::eCONTACT_DEFAULT;

  return physx::PxFilterFlag::eCALLBACK;
}

PandaQueryFilterCallback *PandaQueryFilterCallback::_ptr = nullptr;

/**
 *
 */
physx::PxQueryHitType::Enum PandaQueryFilterCallback::
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

/**
 *
 */
physx::PxQueryHitType::Enum PandaQueryFilterCallback::
postFilter(const physx::PxFilterData &filter_data, const physx::PxQueryHit &hit) {
  return physx::PxQueryHitType::eNONE;
}

/**
 *
 */
PandaQueryFilterCallback *PandaQueryFilterCallback::
ptr() {
  if (_ptr == nullptr) {
    _ptr = new PandaQueryFilterCallback;
  }

  return _ptr;
}

PandaSimulationFilterCallback *PandaSimulationFilterCallback::_ptr = nullptr;


/**
 *
 */
physx::PxFilterFlags PandaSimulationFilterCallback::
pairFound(physx::PxU32 pair_id, physx::PxFilterObjectAttributes attributes0,
          physx::PxFilterData filter_data0, const physx::PxActor *a0,
          const physx::PxShape *shape0,
          physx::PxFilterObjectAttributes attributes1,
          physx::PxFilterData filter_data1, const physx::PxActor *a1,
          const physx::PxShape *shape1,
          physx::PxPairFlags &pair_flags) {

  if (a0->userData == nullptr || a1->userData == nullptr) {
    return physx::PxFilterFlag::eDEFAULT;
  }

  PhysRigidActorNode *node0 = (PhysRigidActorNode *)a0->userData;
  PhysRigidActorNode *node1 = (PhysRigidActorNode *)a1->userData;

  if (node0->has_no_collide_with(node1)) {
    return physx::PxFilterFlag::eSUPPRESS;
  }

  return physx::PxFilterFlag::eDEFAULT;
}

/**
 *
 */
void PandaSimulationFilterCallback::
pairLost(physx::PxU32 pairID,
         physx::PxFilterObjectAttributes attributes0,
         physx::PxFilterData filterData0,
         physx::PxFilterObjectAttributes attributes1,
         physx::PxFilterData filterData1,
         bool objectRemoved) {
}

/**
 *
 */
bool PandaSimulationFilterCallback::
statusChange(physx::PxU32& pairID, physx::PxPairFlags& pairFlags,
             physx::PxFilterFlags& filterFlags) {
  return false;
}

/**
 *
 */
PandaSimulationFilterCallback *PandaSimulationFilterCallback::
ptr() {
  if (_ptr == nullptr) {
    _ptr = new PandaSimulationFilterCallback;
  }

  return _ptr;
}
