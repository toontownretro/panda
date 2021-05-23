/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physXSimulationEventCallback.cxx
 * @author brian
 * @date 2021-04-20
 */

#include "physXSimulationEventCallback.h"
#include "physRigidActorNode.h"
#include "eventStorePandaNode.h"
#include "eventParameter.h"
#include "event.h"
#include "eventQueue.h"
#include "physTriggerCallbackData.h"
#include "physScene.h"
#include "physSleepStateCallbackData.h"
#include "physContactCallbackData.h"

/**
 *
 */
void PhysXSimulationEventCallback::
onConstraintBreak(physx::PxConstraintInfo *constraints, physx::PxU32 count) {
  for (physx::PxU32 i = 0; i < count; i++) {
    physx::PxConstraintInfo &info = constraints[i];
  }
}

/**
 *
 */
void PhysXSimulationEventCallback::
onWake(physx::PxActor **actors, physx::PxU32 count) {
  for (physx::PxU32 i = 0; i < count; i++) {
    physx::PxActor *actor = actors[i];
    if (actor->userData == nullptr) {
      continue;
    }

    PhysRigidActorNode *node = (PhysRigidActorNode *)actor->userData;
    if (node->get_wake_callback() != nullptr) {
      PhysSleepStateCallbackData *cbdata = new PhysSleepStateCallbackData(
        actor, PhysSleepStateCallbackData::S_awake);
      _scene->enqueue_callback(node->get_wake_callback(), cbdata);
    }
  }
}

/**
 *
 */
void PhysXSimulationEventCallback::
onSleep(physx::PxActor **actors, physx::PxU32 count) {
  for (physx::PxU32 i = 0; i < count; i++) {
    physx::PxActor *actor = actors[i];
    if (actor->userData == nullptr) {
      continue;
    }

    PhysRigidActorNode *node = (PhysRigidActorNode *)actor->userData;
    if (node->get_sleep_callback() != nullptr) {
      PhysSleepStateCallbackData *cbdata = new PhysSleepStateCallbackData(
        actor, PhysSleepStateCallbackData::S_asleep);
      _scene->enqueue_callback(node->get_sleep_callback(), cbdata);
    }
  }
}

/**
 *
 */
void PhysXSimulationEventCallback::
onContact(const physx::PxContactPairHeader &pair_header,
          const physx::PxContactPair *pairs, physx::PxU32 num_pairs) {
  if (pair_header.flags.isSet(physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0) ||
      pair_header.flags.isSet(physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1)) {
    return;
  }

  if (pairs == nullptr) {
    return;
  }

  PhysRigidActorNode *node_a = (PhysRigidActorNode *)pair_header.actors[0]->userData;
  PhysRigidActorNode *node_b = (PhysRigidActorNode *)pair_header.actors[1]->userData;

  if (node_a == nullptr || node_b == nullptr) {
    return;
  }

  if (node_a->get_contact_callback() != nullptr ||
      node_b->get_contact_callback() != nullptr) {
    PhysContactCallbackData *cbdata = new PhysContactCallbackData(pair_header);

    if (node_a->get_contact_callback() != nullptr) {
      _scene->enqueue_callback(node_a->get_contact_callback(), cbdata);
    }

    if (node_b->get_contact_callback() != nullptr) {
      _scene->enqueue_callback(node_b->get_contact_callback(), cbdata);
    }
  }
}

/**
 *
 */
void PhysXSimulationEventCallback::
onTrigger(physx::PxTriggerPair *pairs, physx::PxU32 count) {
  for (physx::PxU32 i = 0; i < count; i++) {
    physx::PxTriggerPair &pair = pairs[i];

    if (pair.flags.isSet(physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER) ||
        pair.flags.isSet(physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER)) {
      continue;
    }

    if (pair.triggerActor->userData == nullptr ||
        pair.otherActor->userData == nullptr) {
      continue;
    }

    PhysRigidActorNode *trigger_node = (PhysRigidActorNode *)pair.triggerActor->userData;

    if (trigger_node->get_trigger_callback() != nullptr) {
      PhysTriggerCallbackData *cbdata = new PhysTriggerCallbackData(pair);
      _scene->enqueue_callback(trigger_node->get_trigger_callback(), cbdata);
    }
  }
}

/**
 *
 */
void PhysXSimulationEventCallback::
onAdvance(const physx::PxRigidBody *const *bodyBuffer,
          const physx::PxTransform *poseBuffer, const physx::PxU32 count) {
}
