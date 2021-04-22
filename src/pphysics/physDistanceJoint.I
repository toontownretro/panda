/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physDistanceJoint.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PN_stdfloat PhysDistanceJoint::
get_distance() const {
  return _joint->getDistance();
}

/**
 *
 */
INLINE void PhysDistanceJoint::
set_min_distance(PN_stdfloat distance) {
  _joint->setMinDistance(distance);
  _joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, true);
}

/**
 *
 */
INLINE PN_stdfloat PhysDistanceJoint::
get_min_distance() const {
  return _joint->getMinDistance();
}

/**
 *
 */
INLINE bool PhysDistanceJoint::
has_min_distance() const {
  return _joint->getDistanceJointFlags().isSet(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED);
}

/**
 *
 */
INLINE void PhysDistanceJoint::
clear_min_distance() {
  _joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, false);
}

/**
 *
 */
INLINE void PhysDistanceJoint::
set_max_distance(PN_stdfloat distance) {
  _joint->setMaxDistance(distance);
  _joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
}

/**
 *
 */
INLINE PN_stdfloat PhysDistanceJoint::
get_max_distance() const {
  return _joint->getMaxDistance();
}

/**
 *
 */
INLINE bool PhysDistanceJoint::
has_max_distance() const {
  return _joint->getDistanceJointFlags().isSet(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED);
}

/**
 *
 */
INLINE void PhysDistanceJoint::
clear_max_distance() {
  _joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, false);
}

/**
 *
 */
INLINE void PhysDistanceJoint::
set_tolerance(PN_stdfloat tolerance) {
  _joint->setTolerance(tolerance);
}

/**
 *
 */
INLINE PN_stdfloat PhysDistanceJoint::
get_tolerance() const {
  return _joint->getTolerance();
}

/**
 *
 */
INLINE void PhysDistanceJoint::
set_spring(bool flag) {
  _joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, flag);
}

/**
 *
 */
INLINE bool PhysDistanceJoint::
get_spring() const {
  return _joint->getDistanceJointFlags().isSet(physx::PxDistanceJointFlag::eSPRING_ENABLED);
}

/**
 *
 */
INLINE void PhysDistanceJoint::
set_stiffness(PN_stdfloat stiffness) {
  _joint->setStiffness(stiffness);
}

/**
 *
 */
INLINE PN_stdfloat PhysDistanceJoint::
get_stiffness() const {
  return _joint->getStiffness();
}

/**
 *
 */
INLINE void PhysDistanceJoint::
set_damping(PN_stdfloat damping) {
  _joint->setDamping(damping);
}

/**
 *
 */
INLINE PN_stdfloat PhysDistanceJoint::
get_damping() const {
  return _joint->getDamping();
}
