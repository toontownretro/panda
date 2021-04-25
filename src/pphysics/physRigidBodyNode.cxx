/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidBodyNode.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physRigidBodyNode.h"

TypeHandle PhysRigidBodyNode::_type_handle;

/**
 *
 */
PhysRigidBodyNode::
PhysRigidBodyNode(const std::string &name) :
  PhysRigidActorNode(name)
{
}

/**
 * Automatically computes the mass, center of mass, and inertia tensor of the
 * rigid body from the attached shapes.
 */
void PhysRigidBodyNode::
compute_mass_properties() {
  physx::PxU32 num_shapes = get_rigid_body()->getNbShapes();
  physx::PxShape **shapes = (physx::PxShape **)alloca(sizeof(physx::PxShape *) * num_shapes);
  get_rigid_body()->getShapes(shapes, num_shapes);
  physx::PxMassProperties props = physx::PxRigidBodyExt::computeMassPropertiesFromShapes(shapes, num_shapes);
  get_rigid_body()->setCMassLocalPose(physx::PxTransform(props.centerOfMass));
  get_rigid_body()->setMass(props.mass);
  get_rigid_body()->setMassSpaceInertiaTensor(physx::PxMassProperties::getMassSpaceInertia(props.inertiaTensor, physx::PxQuat()));
}

/**
 *
 */
physx::PxRigidActor *PhysRigidBodyNode::
get_rigid_actor() const {
  return get_rigid_body();
}
