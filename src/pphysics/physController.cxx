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
#include "physQueryFilter.h"
#include "physx_shaders.h"

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
bool PhysControllerFilterCallback::
filter(const physx::PxController &a, const physx::PxController &b) {
  if (a.getUserData() == nullptr ||
      b.getUserData() == nullptr) {
    return true;
  }

  PhysController *pa = (PhysController *)a.getUserData();
  PhysController *pb = (PhysController *)b.getUserData();

  // If they both belong to a collision group, check that first.
  if (pa->get_collision_group() != 0 && pb->get_collision_group() != 0) {
    if (!(PandaSimulationFilterShader::_collision_table[pa->get_collision_group()][pb->get_collision_group()]._enable_collisions)) {
      // The groups don't collide with each other.
      return false;
    }
  }

  // If controller B is solid to controller A, they collide.
  return (pa->get_solid_mask() & pb->get_contents_mask()) != 0;
}

/**
 *
 */
PhysController::CollisionFlags PhysController::
move(double dt, const LVector3 &move_vector, PN_stdfloat min_distance,
     PhysBaseQueryFilter *filter) {

  // Clear out existing hits.
  _shape_hits.clear();
  _controller_hits.clear();
  _collision_flags = CF_none;

  // Lay out the filter data the way that PhysBaseQueryFilter expects.
  physx::PxFilterData fdata;
  fdata.word0 = _solid_mask.get_word();
  fdata.word1 = fdata.word0;
  fdata.word2 = 0;
  fdata.word3 = _collision_group;

  physx::PxControllerFilters filters;
  PhysBaseQueryFilter default_filter;
  PhysControllerFilterCallback controller_filter;
  if (filter == nullptr) {
    filters.mFilterCallback = &default_filter;
  } else {
    filters.mFilterCallback = filter;
  }
  filters.mCCTFilterCallback = &controller_filter;
  filters.mFilterData = &fdata;

  // Run the move.
  uint32_t flags = get_controller()->move(
    Vec3_to_PxVec3(move_vector), min_distance, dt, filters);
  _collision_flags = flags;
  return (CollisionFlags)flags;
}
