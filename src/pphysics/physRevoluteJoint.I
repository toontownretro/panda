/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRevoluteJoint.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_angle() const {
  return rad_2_deg(_joint->getAngle());
}

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_velocity() const {
  return physx_length_to_panda(_joint->getVelocity());
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_limit(const PhysJointLimitAngularPair &limit) {
  _joint->setLimit(limit.get_limit_pair());
  _joint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);
}

/**
 *
 */
INLINE PhysJointLimitAngularPair PhysRevoluteJoint::
get_limit() const {
  return PhysJointLimitAngularPair(_joint->getLimit());
}

/**
 *
 */
INLINE bool PhysRevoluteJoint::
has_limit() const {
  return _joint->getRevoluteJointFlags().isSet(physx::PxRevoluteJointFlag::eLIMIT_ENABLED);
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
clear_limit() {
  _joint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, false);
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_drive(bool flag) {
  _joint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_ENABLED, flag);
}

/**
 *
 */
INLINE bool PhysRevoluteJoint::
get_drive() const {
  return _joint->getRevoluteJointFlags().isSet(physx::PxRevoluteJointFlag::eDRIVE_ENABLED);
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_drive_freespin(bool flag) {
  _joint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eDRIVE_FREESPIN, flag);
}

/**
 *
 */
INLINE bool PhysRevoluteJoint::
get_drive_freespin() const {
  return _joint->getRevoluteJointFlags().isSet(physx::PxRevoluteJointFlag::eDRIVE_FREESPIN);
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_drive_velocity(PN_stdfloat vel, bool autowake) {
  _joint->setDriveVelocity(panda_length_to_physx(vel), autowake);
}

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_drive_velocity() const {
  return _joint->getDriveVelocity();
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_drive_force_limit(PN_stdfloat limit) {
  _joint->setDriveForceLimit(panda_length_to_physx(limit));
}

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_drive_force_limit() const {
  return physx_length_to_panda(_joint->getDriveForceLimit());
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_drive_gear_ratio(PN_stdfloat ratio) {
  _joint->setDriveGearRatio(ratio);
}

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_drive_gear_ratio() const {
  return _joint->getDriveGearRatio();
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_projection_linear_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionLinearTolerance(panda_length_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_projection_linear_tolerance() const {
  return physx_length_to_panda(_joint->getProjectionLinearTolerance());
}

/**
 *
 */
INLINE void PhysRevoluteJoint::
set_projection_angular_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionAngularTolerance(panda_ang_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysRevoluteJoint::
get_projection_angular_tolerance() const {
  return physx_ang_to_panda(_joint->getProjectionAngularTolerance());
}
