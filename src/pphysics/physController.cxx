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
  data._motion_dir = physx_norm_vec_to_panda(hit.dir);
  data._motion_length = physx_length_to_panda(hit.length);
  data._triangle_index = (uint32_t)hit.triangleIndex;
  data._world_normal = physx_norm_vec_to_panda(hit.worldNormal);
  data._world_pos = physx_ex_vec_to_panda(hit.worldPos);

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
  data._world_pos = physx_ex_vec_to_panda(hit.worldPos);
  data._world_normal = physx_norm_vec_to_panda(hit.worldNormal);
  data._motion_dir = physx_norm_vec_to_panda(hit.dir);
  data._motion_length = physx_length_to_panda(hit.length);

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
  PhysRigidDynamicNode *aa = pa->get_actor_node();
  PhysRigidDynamicNode *ab = pb->get_actor_node();

  // If one of them is not solid to the other, they don't collide.
  return ((aa->get_from_collide_mask() & ab->get_into_collide_mask()) != 0) &&
         ((ab->get_from_collide_mask() & aa->get_into_collide_mask()) != 0);
}

/**
 * Sets the mask of contents of the controller.  This is AND'd against the
 * solid mask of other controllers to determine if two controllers should
 * collide or pass through each other.
 */
void PhysController::
set_from_collide_mask(BitMask32 mask) {
  nassertv(_actor_node != nullptr);
  _actor_node->set_from_collide_mask(mask);
}

/**
 *
 */
PhysController::CollisionFlags PhysController::
move(double dt, const LVector3 &move_vector, PN_stdfloat min_distance,
     BitMask32 collide_mask,
     CallbackObject *filter) {

  // Clear out existing hits.
  _shape_hits.clear();
  _controller_hits.clear();
  _collision_flags = CF_none;

  // Lay out the filter data the way that PhysBaseQueryFilter expects.
  physx::PxFilterData fdata;
  fdata.word0 = collide_mask.get_word();
  fdata.word1 = fdata.word0;
  fdata.word2 = 0;

  physx::PxControllerFilters filters;
  PhysBaseQueryFilter pfilter(filter);
  filters.mFilterCallback = &pfilter;
  PhysControllerFilterCallback controller_filter;
  filters.mCCTFilterCallback = &controller_filter;
  filters.mFilterData = &fdata;

  // Run the move.
  uint32_t flags = get_controller()->move(
    panda_vec_to_physx(move_vector), panda_length_to_physx(min_distance), dt, filters);
  _collision_flags = flags;
  return (CollisionFlags)flags;
}

/**
 *
 */
void PhysController::
resize(PN_stdfloat size) {
  get_controller()->resize(panda_length_to_physx(size));
}
