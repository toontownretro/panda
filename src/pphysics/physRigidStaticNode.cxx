/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidStaticNode.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physRigidStaticNode.h"
#include "physSystem.h"

TypeHandle PhysRigidStaticNode::_type_handle;

/**
 *
 */
PhysRigidStaticNode::
PhysRigidStaticNode(const std::string &name) :
  PhysRigidActorNode(name)
{
  PhysSystem *sys = PhysSystem::ptr();
  _rigid_static = sys->get_physics()->createRigidStatic(
    physx::PxTransform(physx::PxVec3(0, 0, 0)));
  _rigid_static->userData = this;
}

/**
 *
 */
PhysRigidStaticNode::
~PhysRigidStaticNode() {
  if (_rigid_static != nullptr) {
    _rigid_static->userData = nullptr;

    if (_rigid_static->getScene() != nullptr) {
      _rigid_static->getScene()->removeActor(*_rigid_static);
    }

    _rigid_static->release();
    _rigid_static = nullptr;
  }
}
/**
 *
 */
physx::PxRigidActor *PhysRigidStaticNode::
get_rigid_actor() const {
  return _rigid_static;
}
