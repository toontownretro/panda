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
 *
 */
INLINE void PhysRigidBodyNode::
add_force(const LVector3 &force, bool auto_wake) {
  get_rigid_body()->addForce(
    physx::PxVec3(force[0], force[1], force[2]),
    physx::PxForceMode::eFORCE, auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
clear_force() {
  get_rigid_body()->clearForce(physx::PxForceMode::eFORCE);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_impulse_force(const LVector3 &force, bool auto_wake) {
  get_rigid_body()->addForce(
    physx::PxVec3(force[0], force[1], force[2]),
    physx::PxForceMode::eIMPULSE, auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
clear_impulse_force() {
  get_rigid_body()->clearForce(physx::PxForceMode::eIMPULSE);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_torque(const LVector3 &torque, bool auto_wake) {
  get_rigid_body()->addTorque(
    physx::PxVec3(torque[0], torque[1], torque[2]),
    physx::PxForceMode::eFORCE, auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
clear_torque() {
  get_rigid_body()->clearTorque(physx::PxForceMode::eFORCE);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_impulse_torque(const LVector3 &torque, bool auto_wake) {
  get_rigid_body()->addTorque(
    physx::PxVec3(torque[0], torque[1], torque[2]),
    physx::PxForceMode::eIMPULSE, auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
clear_impulse_torque() {
  get_rigid_body()->clearTorque(physx::PxForceMode::eIMPULSE);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_min_ccd_advance_coefficient(PN_stdfloat coef) {
  get_rigid_body()->setMinCCDAdvanceCoefficient(coef);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_min_ccd_advance_coefficient() const {
  return get_rigid_body()->getMinCCDAdvanceCoefficient();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_max_depenetration_velocity(PN_stdfloat bias_clamp) {
  get_rigid_body()->setMaxDepenetrationVelocity(bias_clamp);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_depenetration_velocity() const {
  return get_rigid_body()->getMaxDepenetrationVelocity();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_max_contact_impulse(PN_stdfloat max) {
  get_rigid_body()->setMaxContactImpulse(max);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_contact_impulse() const {
  return get_rigid_body()->getMaxContactImpulse();
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_ccd_enabled(bool flag) {
  get_rigid_body()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, flag);
}

/**
 *
 */
INLINE bool PhysRigidBodyNode::
get_ccd_enabled() const {
  return get_rigid_body()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eENABLE_CCD);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_retain_accelerations(bool flag) {
  get_rigid_body()->setRigidBodyFlag(physx::PxRigidBodyFlag::eRETAIN_ACCELERATIONS, flag);
}

/**
 *
 */
INLINE bool PhysRigidBodyNode::
get_retain_accelerations() const {
  return get_rigid_body()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eRETAIN_ACCELERATIONS);
}

/**
 * Sets whether or not this is a kinematic object, meaning that it's position
 * and orientation is controlled without regard to mass, velocity, etc.  Mainly
 * used for things like avatars.
 */
INLINE void PhysRigidBodyNode::
set_kinematic(bool flag) {
  get_rigid_body()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, flag);
  get_rigid_body()->setRigidBodyFlag(physx::PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES, flag);
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
