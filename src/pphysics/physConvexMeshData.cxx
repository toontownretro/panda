/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physConvexMeshData.cxx
 * @author brian
 * @date 2021-04-22
 */

#include "physConvexMeshData.h"
#include "physSystem.h"
#include "config_pphysics.h"

/**
 *
 */
bool PhysConvexMeshData::
generate_mesh() {
  invalidate_mesh();

  PhysSystem *sys = PhysSystem::ptr();
  physx::PxPhysics *physics = sys->get_physics();
  physx::PxCooking *cooking = sys->get_cooking();

  physx::PxConvexMeshDesc desc;
  desc.points.count = _points.size();
  desc.points.stride = sizeof(physx::PxVec3);
  desc.points.data = _points.data();
  desc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

  // Validation doesn't seem to work, but cooking does... not sure why.
//#ifndef NDEBUG
  // Make sure it's a valid mesh.
  //if (!cooking->validateConvexMesh(desc)) {
  //  pphysics_cat.error()
  //    << "Failed to validate convex mesh desc\n";
  //  return false;
  //}
//#endif

  physx::PxConvexMeshCookingResult::Enum result;
  _mesh = cooking->createConvexMesh(desc, physics->getPhysicsInsertionCallback(), &result);

  return (_mesh != nullptr) && (result == physx::PxConvexMeshCookingResult::eSUCCESS);
}
