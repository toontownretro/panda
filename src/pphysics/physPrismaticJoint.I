/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physPrismaticJoint.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PN_stdfloat PhysPrismaticJoint::
get_position() const {
  return _joint->getPosition();
}

/**
 *
 */
INLINE PN_stdfloat PhysPrismaticJoint::
get_velocity() const {
  return _joint->getVelocity();
}

/**
 *
 */
INLINE void PhysPrismaticJoint::
set_limit(const PhysJointLimitLinearPair &limit) {
  _joint->setLimit(limit.get_limit_pair());
  _joint->setPrismaticJointFlag(physx::PxPrismaticJointFlag::eLIMIT_ENABLED, true);
}

/**
 *
 */
INLINE PhysJointLimitLinearPair PhysPrismaticJoint::
get_limit() const {
  return PhysJointLimitLinearPair(_joint->getLimit());
}

/**
 *
 */
INLINE bool PhysPrismaticJoint::
has_limit() const {
  return _joint->getPrismaticJointFlags().isSet(physx::PxPrismaticJointFlag::eLIMIT_ENABLED);
}

/**
 *
 */
INLINE void PhysPrismaticJoint::
clear_limit() {
  _joint->setPrismaticJointFlag(physx::PxPrismaticJointFlag::eLIMIT_ENABLED, false);
}

/**
 *
 */
INLINE void PhysPrismaticJoint::
set_projection_linear_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionLinearTolerance(tolerance);
}

/**
 *
 */
INLINE PN_stdfloat PhysPrismaticJoint::
get_projection_linear_tolerance() const {
  return _joint->getProjectionLinearTolerance();
}

/**
 *
 */
INLINE void PhysPrismaticJoint::
set_projection_angular_tolerance(PN_stdfloat tolerance) {
  _joint->setProjectionAngularTolerance(tolerance);
}

/**
 *
 */
INLINE PN_stdfloat PhysPrismaticJoint::
get_projection_angular_tolerance() const {
  return _joint->getProjectionAngularTolerance();
}
