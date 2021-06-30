/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physFixedJoint.I
 * @author brian
 * @date 2021-04-21
 */

/**
 *
 */
INLINE void PhysFixedJoint::
set_projection_linear_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionLinearTolerance(panda_length_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysFixedJoint::
get_projection_linear_tolerance() const {
  return physx_length_to_panda(_joint->getProjectionLinearTolerance());
}

/**
 *
 */
INLINE void PhysFixedJoint::
set_projection_angular_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionAngularTolerance(panda_ang_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysFixedJoint::
get_projection_angular_tolerance() const {
  return physx_ang_to_panda(_joint->getProjectionAngularTolerance());
}
