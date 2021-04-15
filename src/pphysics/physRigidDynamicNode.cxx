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
#include "nodePath.h"
#include "physSystem.h"

#include "physx_includes.h"

TypeHandle PhysRigidDynamicNode::_type_handle;

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
 * Called when something other than the PhysX simulation caused the transform
 * of the node to change.  Synchronizes the node's new transform with the
 * associated PhysX actor.
 */
void PhysRigidDynamicNode::
do_transform_changed() {
  if (!is_kinematic()) {
    // Not a kinematic actor, just use the default behavior.
    PhysRigidBodyNode::do_transform_changed();
    return;
  }

  if (!get_sync_enabled()) {
    return;
  }

  NodePath np = NodePath::any_path((PandaNode *)this);

  CPT(TransformState) net_transform = np.get_net_transform();
  const LPoint3 &pos = net_transform->get_pos();
  const LQuaternion &quat = net_transform->get_quat();

  physx::PxTransform pose(
    physx::PxVec3(pos[0], pos[1], pos[2]),
    physx::PxQuat(quat[1], quat[2], quat[3], quat[0]));

  _rigid_dynamic->setKinematicTarget(pose);
}

/**
 *
 */
physx::PxRigidBody *PhysRigidDynamicNode::
get_rigid_body() const {
  return _rigid_dynamic;
}
