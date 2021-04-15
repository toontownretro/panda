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

/**
 *
 */
PhysRigidBodyNode::
PhysRigidBodyNode(const std::string &name) :
  PhysRigidActorNode(name)
{
}

/**
 *
 */
physx::PxRigidActor *PhysRigidBodyNode::
get_rigid_actor() const {
  return get_rigid_body();
}
