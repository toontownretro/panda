/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physFixedJoint.cxx
 * @author brian
 * @date 2021-04-21
 */

#include "physFixedJoint.h"
#include "physSystem.h"

/**
 *
 */
PhysFixedJoint::
PhysFixedJoint(PhysRigidActorNode *a, PhysRigidActorNode *b,
               const TransformState *frame_a, const TransformState *frame_b) {
  PhysSystem *sys = PhysSystem::ptr();
  _joint = physx::PxFixedJointCreate(
    *sys->get_physics(),
    a->get_rigid_actor(), panda_trans_to_physx(frame_a),
    b->get_rigid_actor(), panda_trans_to_physx(frame_b));
  _a = a;
  _b = b;
}

/**
 *
 */
PhysFixedJoint::
~PhysFixedJoint() {
  if (_joint != nullptr) {
    _joint->userData = nullptr;
    _joint->release();
    _joint = nullptr;
  }
}

/**
 *
 */
physx::PxJoint *PhysFixedJoint::
get_joint() const {
  return _joint;
}
