/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidBodyNode.I
 * @author brian
 * @date 2021-04-14
 */

/**
 * Sets the mass of the rigid body.
 */
INLINE void PhysRigidBodyNode::
set_mass(PN_stdfloat mass) {
  get_rigid_body()->setMass((physx::PxReal)mass);
}

/**
 * Returns the mass of the rigid body.
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_mass() const {
  return (PN_stdfloat)get_rigid_body()->getMass();
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_inv_mass() const {
  return (PN_stdfloat)get_rigid_body()->getInvMass();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_linear_damping(PN_stdfloat damping) {
  get_rigid_body()->setLinearDamping((physx::PxReal)damping);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_linear_damping() const {
  return (PN_stdfloat)get_rigid_body()->getLinearDamping();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_angular_damping(PN_stdfloat damping) {
  get_rigid_body()->setAngularDamping((physx::PxReal)damping);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_angular_damping() const {
  return (PN_stdfloat)get_rigid_body()->getAngularDamping();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_linear_velocity(const LVector3 &vel, bool auto_wake) {
  get_rigid_body()->setLinearVelocity(
    physx::PxVec3(vel[0], vel[1], vel[2]), auto_wake);
}

/**
 *
 */
INLINE LVector3 PhysRigidBodyNode::
get_linear_velocity() const {
  physx::PxVec3 vel = get_rigid_body()->getLinearVelocity();
  return LVector3(vel[0], vel[1], vel[2]);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_max_linear_velocity(PN_stdfloat max) {
  get_rigid_body()->setMaxLinearVelocity((physx::PxReal)max);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_linear_velocity() const {
  return (PN_stdfloat)get_rigid_body()->getMaxLinearVelocity();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_angular_velocity(const LVector3 &vel, bool auto_wake) {
  get_rigid_body()->setAngularVelocity(
    physx::PxVec3(vel[0], vel[1], vel[2]), auto_wake);
}

/**
 *
 */
INLINE LVector3 PhysRigidBodyNode::
get_angular_velocity() const {
  physx::PxVec3 vel = get_rigid_body()->getAngularVelocity();
  return LVector3(vel[0], vel[1], vel[2]);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_max_angular_velocity(PN_stdfloat max) {
  get_rigid_body()->setMaxAngularVelocity(max);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_angular_velocity() const {
  return get_rigid_body()->getMaxAngularVelocity();
}

/**
 * Sets whether or not this is a kinematic object, meaning that it's position
 * and orientation is controlled without regard to mass, velocity, etc.  Mainly
 * used for things like avatars.
 */
INLINE void PhysRigidBodyNode::
set_kinematic(bool flag) {
  get_rigid_body()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, flag);
}

/**
 * Returns whether or not this is a kinematic object, meaning that it's
 * position and orientation is controlled without regard to mass, velocity,
 * etc.  See set_kinematic().
 */
INLINE bool PhysRigidBodyNode::
is_kinematic() const {
  return get_rigid_body()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
}
