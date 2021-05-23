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

/**
 *
 */
PhysController::CollisionFlags PhysController::
move(double dt, const LVector3 &move_vector,  PN_stdfloat min_distance) {
  physx::PxFilterData fdata;
  fdata.word0 = _group_mask.get_word();
  fdata.word1 = fdata.word0;
  physx::PxControllerFilters filters;
  filters.mFilterData = &fdata;

  uint32_t flags = get_controller()->move(
    Vec3_to_PxVec3(move_vector), min_distance, dt, filters);
  return (CollisionFlags)flags;
}
