/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidDynamicNode.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physRigidDynamicNode.h"

#include "physSystem.h"

#include "physx_includes.h"

/**
 *
 */
PhysRigidDynamicNode::
PhysRigidDynamicNode(const std::string &name) :
  PhysRigidBodyNode(name)
{
  PhysSystem *sys = PhysSystem::ptr();
  _rigid_dynamic = sys->get_physics()->createRigidDynamic(
    physx::PxTransform(physx::PxVec3(0, 0, 0)));
  _rigid_dynamic->userData = this;
}

/**
 *
 */
PhysRigidDynamicNode::
~PhysRigidDynamicNode() {
  if (_rigid_dynamic != nullptr) {

    // Not sure if this is done automatically by PhysX, so I'm doing it just
    // to be safe.
    if (_rigid_dynamic->getScene() != nullptr) {
      _rigid_dynamic->getScene()->removeActor(*_rigid_dynamic);
    }

    _rigid_dynamic->release();
    _rigid_dynamic = nullptr;
  }
}

/**
 *
 */
physx::PxRigidBody *PhysRigidDynamicNode::
get_rigid_body() const {
  return _rigid_dynamic;
}
