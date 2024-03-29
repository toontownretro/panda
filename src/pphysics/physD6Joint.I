/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physD6Joint.I
 * @author brian
 * @date 2021-04-22
 */

/**
 * Sets the linear motion type for the specified axis.
 */
INLINE void PhysD6Joint::
set_linear_motion(Axis axis, Motion motion) {
  _joint->setMotion((physx::PxD6Axis::Enum)axis, (physx::PxD6Motion::Enum)motion);
}

/**
 *
 */
INLINE PhysD6Joint::Motion PhysD6Joint::
get_linear_motion(Axis axis) const {
  return (Motion)_joint->getMotion((physx::PxD6Axis::Enum)axis);
}

/**
 *
 */
INLINE void PhysD6Joint::
set_angular_motion(Axis axis, Motion motion) {
  _joint->setMotion((physx::PxD6Axis::Enum)(axis + 3), (physx::PxD6Motion::Enum)motion);
}

/**
 *
 */
INLINE PhysD6Joint::Motion PhysD6Joint::
get_angular_motion(Axis axis) const {
  return (Motion)_joint->getMotion((physx::PxD6Axis::Enum)(axis + 3));
}

/**
 *
 */
INLINE void PhysD6Joint::
set_linear_limit(Axis axis, const PhysJointLimitLinearPair &limit) {
  _joint->setLinearLimit((physx::PxD6Axis::Enum)axis, limit.get_limit_pair());
}

/**
 *
 */
INLINE PhysJointLimitLinearPair PhysD6Joint::
get_linear_limit(Axis axis) const {
  return PhysJointLimitLinearPair(_joint->getLinearLimit((physx::PxD6Axis::Enum)axis));
}

/**
 *
 */
INLINE void PhysD6Joint::
set_twist_limit(const PhysJointLimitAngularPair &limit) {
  _joint->setTwistLimit(limit.get_limit_pair());
}

/**
 *
 */
INLINE PhysJointLimitAngularPair PhysD6Joint::
get_twist_limit() const {
  return PhysJointLimitAngularPair(_joint->getTwistLimit());
}

/**
 *
 */
INLINE void PhysD6Joint::
set_swing_limit(const PhysJointLimitCone &limit) {
  _joint->setSwingLimit(limit.get_limit_cone());
}

/**
 *
 */
INLINE PhysJointLimitCone PhysD6Joint::
get_swing_limit() const {
  return PhysJointLimitCone(_joint->getSwingLimit());
}

/**
 *
 */
INLINE void PhysD6Joint::
set_pyramid_swing_limit(const PhysJointLimitPyramid &limit) {
  _joint->setPyramidSwingLimit(limit.get_limit_pyramid());
}

/**
 *
 */
INLINE PhysJointLimitPyramid PhysD6Joint::
get_pyramid_swing_limit() const {
  return PhysJointLimitPyramid(_joint->getPyramidSwingLimit());
}

/**
 *
 */
INLINE void PhysD6Joint::
set_projection_linear_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionLinearTolerance(panda_length_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysD6Joint::
get_projection_linear_tolerance() const {
  return physx_length_to_panda(_joint->getProjectionLinearTolerance());
}

/**
 *
 */
INLINE void PhysD6Joint::
set_projection_angular_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionAngularTolerance(panda_ang_to_physx(tolerance));
}

/**
 *
 */
INLINE PN_stdfloat PhysD6Joint::
get_projection_angular_tolerance() const {
  return physx_ang_to_panda(_joint->getProjectionAngularTolerance());
}

/**
 *
 */
INLINE PN_stdfloat PhysD6Joint::
get_angle(Axis axis) const {
  switch (axis) {
  case A_x:
    return physx_ang_to_panda(_joint->getTwistAngle());
  case A_y:
    return physx_ang_to_panda(_joint->getSwingYAngle());
  case A_z:
    return physx_ang_to_panda(_joint->getSwingZAngle());
  }
}
