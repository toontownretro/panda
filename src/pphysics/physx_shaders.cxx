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
 * Returns true if the two collision groups have collisions enabled, false
 * otherwise.
 */
static bool
should_collision_groups_collide(int group0, int group1) {
  if (group0 == 0 || group1 == 0) {
    // One or both of the shapes are not assigned to any collision groups.
    // Collisions will always occur.
    return true;

  } else {
    // Both shapes are assigned to a collision group.  Check the collision
    // table to see if the groups the shapes belong to have collisions enabled.
    return PandaSimulationFilterShader::_collision_table[group0][group1]._enable_collisions;
  }
}

/**
 * Returns true if the two shapes should collide based on the contents and
 * solid masks of each shape.
 */
static bool
should_contents_collide(int contents0, int solid0, int contents1, int solid1) {
  // If one of them is not solid to the other, they don't collide.
  return ((contents0 & solid1) != 0) && ((contents1 & solid0) != 0);
}

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

  // Shape FilterData
  // word0: collision group
  // word1: contents mask
  // word2: solid mask

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

  // Handle triggers.
  if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1)) {
    pair_flags = physx::PxPairFlag::eTRIGGER_DEFAULT;

    if (physx::PxFilterObjectIsTrigger(attributes0)) {
      // Object A is a trigger, check what the trigger is solid to.
      if ((filter_data0.word2 & filter_data1.word1) != 0) {
        // B is solid to trigger A.
        return physx::PxFilterFlag::eDEFAULT;
      } else {
        // Not solid.
        return physx::PxFilterFlag::eSUPPRESS;
      }
    } else {
      // Object B is the trigger, check what it's solid to.
      if ((filter_data1.word2 & filter_data0.word1) != 0) {
        // A is solid to trigger B.
        return physx::PxFilterFlag::eDEFAULT;
      } else {
        // Not solid
        return physx::PxFilterFlag::eSUPPRESS;
      }
    }
  }

  if (!should_collision_groups_collide(filter_data0.word0, filter_data1.word0)) {
    return physx::PxFilterFlag::eSUPPRESS;
  }

  if (!should_contents_collide(filter_data0.word1, filter_data0.word2,
                               filter_data1.word1, filter_data1.word2)) {
    return physx::PxFilterFlag::eSUPPRESS;
  }

  pair_flags = physx::PxPairFlag::eCONTACT_DEFAULT |
               physx::PxPairFlag::eNOTIFY_TOUCH_FOUND |
               physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;

  return physx::PxFilterFlag::eCALLBACK;
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
