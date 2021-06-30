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

#include <ostream>
#include <sstream>
#include "streamWrapper.h"
#include "physXStreams.h"
#include "datagramBuffer.h"

/**
 * Creates a convex mesh from input points.  When cook is false, a convex mesh
 * directly created from the input points and stored on this object, which can
 * then be retrieved and put inside a PhysShape.  When cook is true, the convex
 * mesh is serialized to a buffer that can be written to disk or stored
 * elsewhere, and later be loaded into a new PhysConvexMeshData.
 */
bool PhysConvexMeshData::
generate_mesh() {
  invalidate_mesh();

  PhysSystem *sys = PhysSystem::ptr();
  physx::PxPhysics *physics = sys->get_physics();

  if (has_mesh_data()) {
    // Create from mesh data buffer.

    std::istringstream ss(
      std::string((char *)_mesh_data.p(), _mesh_data.size()));
    IStreamWrapper wrapper(&ss, false);
    PhysXInputStream pstream(wrapper);
    _mesh = physics->createConvexMesh(pstream);
    return (_mesh != nullptr);

  } else {
    // Run-time creation from points.

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
}

/**
 * Fills a buffer of convex mesh data from the input points.  Returns true
 * on success or false if the mesh couldn't be cooked.  Follow up with a call
 * to generate_mesh() to create an actual mesh object from the buffer
 * (if needed).
 */
bool PhysConvexMeshData::
cook_mesh() {
  PhysSystem *sys = PhysSystem::ptr();
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
  // Create and serialize mesh data to binary stream.
  std::ostringstream out;
  OStreamWrapper wrapper(&out, false);
  PhysXOutputStream pstream(wrapper);
  bool ret = cooking->cookConvexMesh(desc, pstream, &result);
  std::string data = out.str();
  PTA_uchar mesh_data = PTA_uchar::empty_array(data.size());
  memcpy(mesh_data.p(), data.c_str(), data.size());
  _mesh_data = mesh_data;
  _has_mesh_data = true;
  return ret && (result == physx::PxConvexMeshCookingResult::eSUCCESS);
}

/**
 *
 */
void PhysConvexMeshData::
get_mass_information(PN_stdfloat *mass, LMatrix3 *inertia_tensor, LPoint3 *center_of_mass) const {
  if (_mesh == nullptr) {
    return;
  }
  physx::PxReal m;
  physx::PxMat33 it;
  physx::PxVec3 com;
  _mesh->getMassInformation(m, it, com);
  if (mass != nullptr) {
    *mass = physx_mass_to_panda(m);
  }
  if (inertia_tensor != nullptr) {
    inertia_tensor->set(it[0][0], it[0][1], it[0][2], it[1][0], it[1][1], it[1][2],
                      it[2][0], it[2][1], it[2][2]);
  }
  if (center_of_mass != nullptr) {
    *center_of_mass = physx_vec_to_panda(com);
  }
}
