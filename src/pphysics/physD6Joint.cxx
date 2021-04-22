/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physD6Joint.cxx
 * @author brian
 * @date 2021-04-22
 */

#include "physD6Joint.h"
#include "physSystem.h"

/**
 *
 */
PhysD6Joint::
PhysD6Joint(PhysRigidActorNode *a, PhysRigidActorNode *b,
            const TransformState *frame_a, const TransformState *frame_b)
{
  PhysSystem *sys = PhysSystem::ptr();
  _joint = physx::PxD6JointCreate(
    *sys->get_physics(),
    a->get_rigid_actor(), TransformState_to_PxTransform(frame_a),
    b->get_rigid_actor(), TransformState_to_PxTransform(frame_b));
}

/**
 *
 */
PhysD6Joint::
~PhysD6Joint() {
  if (_joint != nullptr) {
    _joint->release();
    _joint = nullptr;
  }
}

/**
 *
 */
physx::PxJoint *PhysD6Joint::
get_joint() const {
  return _joint;
}
