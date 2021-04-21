/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriggerCallbackData.I
 * @author brian
 * @date 2021-04-21
 */

/**
 *
 */
INLINE PhysTriggerCallbackData::
PhysTriggerCallbackData(const physx::PxTriggerPair &pair)
{
  _pair = pair;
}

/**
 *
 */
INLINE PhysTriggerCallbackData::Touch PhysTriggerCallbackData::
get_touch_type() const {
  switch (_pair.status) {
  case physx::PxPairFlag::eNOTIFY_TOUCH_FOUND:
    return T_enter;
  case physx::PxPairFlag::eNOTIFY_TOUCH_LOST:
    return T_exit;
  default:
    return T_none;
  }
}

/**
 *
 */
INLINE PhysRigidActorNode *PhysTriggerCallbackData::
get_trigger_node() const {
  if (_pair.triggerActor->userData == nullptr) {
    return nullptr;
  }

  return (PhysRigidActorNode *)_pair.triggerActor->userData;
}

/**
 *
 */
INLINE PhysShape *PhysTriggerCallbackData::
get_trigger_shape() const {
  if (_pair.triggerShape->userData == nullptr) {
    return nullptr;
  }

  return (PhysShape *)_pair.triggerShape->userData;
}

/**
 *
 */
INLINE PhysRigidActorNode *PhysTriggerCallbackData::
get_other_node() const {
  if (_pair.otherActor->userData == nullptr) {
    return nullptr;
  }

  return (PhysRigidActorNode *)_pair.otherActor->userData;
}

/**
 *
 */
INLINE PhysShape *PhysTriggerCallbackData::
get_other_shape() const {
  if (_pair.otherShape->userData == nullptr) {
    return nullptr;
  }

  return (PhysShape *)_pair.otherShape->userData;
}
