/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physController.cxx
 * @author brian
 * @date 2021-04-27
 */

#include "physController.h"

PhysControllerHitCallback *PhysControllerHitCallback::_global_ptr = nullptr;

/**
 * Called when a moving character hits a non-moving world shape.
 */
void PhysControllerHitCallback::
onShapeHit(const physx::PxControllerShapeHit &hit) {
  if (hit.controller == nullptr ||
      hit.controller->getUserData() == nullptr ||
      hit.actor == nullptr ||
      hit.actor->userData == nullptr ||
      hit.shape == nullptr ||
      hit.shape->userData == nullptr) {
    // No controller or PhysController associated with the controller.
    return;
  }

  PhysController *controller = (PhysController *)(hit.controller->getUserData());

  PhysControllerShapeHitData data;
  data._controller = controller;
  data._actor = (PhysRigidActorNode *)(hit.actor->userData);
  data._shape = (PhysShape *)(hit.shape->userData);
  data._motion_dir = PxVec3_to_Vec3(hit.dir);
  data._motion_length = (PN_stdfloat)hit.length;
  data._triangle_index = (uint32_t)hit.triangleIndex;
  data._world_normal = PxVec3_to_Vec3(hit.worldNormal);
  data._world_pos = PxExVec3_to_Vec3(hit.worldPos);

  controller->_shape_hits.push_back(std::move(data));
}

/**
 * Called when a character hits another character.
 */
void PhysControllerHitCallback::
onControllerHit(const physx::PxControllersHit &hit) {
  if (hit.controller == nullptr ||
      hit.controller->getUserData() == nullptr ||
      hit.other == nullptr ||
      hit.other->getUserData() == nullptr) {
    return;
  }

  PhysController *controller = (PhysController *)(hit.controller->getUserData());
  PhysController *other = (PhysController *)(hit.other->getUserData());

  PhysControllersHitData data;
  data._controller = controller;
  data._other = other;
  data._world_pos = PxExVec3_to_Vec3(hit.worldPos);
  data._world_normal = PxVec3_to_Vec3(hit.worldNormal);
  data._motion_dir = PxVec3_to_Vec3(hit.dir);
  data._motion_length = (PN_stdfloat)hit.length;

  controller->_controller_hits.push_back(std::move(data));
}

/**
 * Called when a character hits an obstacle.  Not implemented.
 */
void PhysControllerHitCallback::
onObstacleHit(const physx::PxControllerObstacleHit &hit) {
}

/**
 *
 */
PhysController::CollisionFlags PhysController::
move(double dt, const LVector3 &move_vector,  PN_stdfloat min_distance) {
  // Clear out existing hits.
  _shape_hits.clear();
  _controller_hits.clear();
  _collision_flags = CF_none;

  physx::PxFilterData fdata;
  fdata.word0 = _group_mask.get_word();
  fdata.word1 = fdata.word0;
  physx::PxControllerFilters filters;
  filters.mFilterData = &fdata;

  // Run the move.
  uint32_t flags = get_controller()->move(
    Vec3_to_PxVec3(move_vector), min_distance, dt, filters);
  _collision_flags = flags;
  return (CollisionFlags)flags;
}
