/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSphericalJoint.I
 * @author brian
 * @date 2021-04-21
 */

/**
 *
 */
INLINE void PhysSphericalJoint::
set_limit_cone(const PhysJointLimitCone &limit) {
  _joint->setLimitCone(limit.get_limit_cone());
  _joint->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);
}

/**
 *
 */
INLINE PhysJointLimitCone PhysSphericalJoint::
get_limit_cone() const {
  return PhysJointLimitCone(_joint->getLimitCone());
}

/**
 *
 */
INLINE bool PhysSphericalJoint::
has_limit_cone() const {
  return _joint->getSphericalJointFlags().isSet(physx::PxSphericalJointFlag::eLIMIT_ENABLED);
}

/**
 *
 */
INLINE void PhysSphericalJoint::
clear_limit_cone() {
  _joint->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, false);
}

/**
 *
 */
INLINE PN_stdfloat PhysSphericalJoint::
get_swing_y_angle() const {
  return rad_2_deg(_joint->getSwingYAngle());
}

/**
 *
 */
INLINE PN_stdfloat PhysSphericalJoint::
get_swing_z_angle() const {
  return rad_2_deg(_joint->getSwingZAngle());
}

/**
 *
 */
INLINE void PhysSphericalJoint::
set_projection_linear_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionLinearTolerance(panda_length_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysSphericalJoint::
get_projection_linear_tolerance() const {
  return physx_length_to_panda(_joint->getProjectionLinearTolerance());
}
