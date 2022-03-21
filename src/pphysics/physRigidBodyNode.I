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
 * Sets the mass of the rigid body.  Automatically recomputes inertia and
 * center-of-mass.
 */
INLINE void PhysRigidBodyNode::
set_mass(PN_stdfloat mass) {
  physx::PxRigidBodyExt::setMassAndUpdateInertia(
    *get_rigid_body(), panda_mass_to_physx(mass));
}

/**
 * Returns the mass of the rigid body.
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_mass() const {
  return physx_mass_to_panda(get_rigid_body()->getMass());
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
    panda_vec_to_physx(vel), auto_wake);
}

/**
 *
 */
INLINE LVector3 PhysRigidBodyNode::
get_linear_velocity() const {
  physx::PxVec3 vel = get_rigid_body()->getLinearVelocity();
  return physx_vec_to_panda(vel);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_max_linear_velocity(PN_stdfloat max) {
  get_rigid_body()->setMaxLinearVelocity(panda_length_to_physx(max));
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_linear_velocity() const {
  return physx_length_to_panda(get_rigid_body()->getMaxLinearVelocity());
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_angular_velocity(const LVector3 &vel, bool auto_wake) {
  get_rigid_body()->setAngularVelocity(
    panda_ang_to_physx(vel), auto_wake);
}

/**
 *
 */
INLINE LVector3 PhysRigidBodyNode::
get_angular_velocity() const {
  physx::PxVec3 vel = get_rigid_body()->getAngularVelocity();
  return physx_ang_to_panda(vel);
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
 * Applies a force defined global coordinates to the rigid body's center of
 * mass.
 */
INLINE void PhysRigidBodyNode::
add_force(const LVector3 &global_force, ForceType type, bool auto_wake) {
  get_rigid_body()->addForce(
    panda_vec_to_physx(global_force),
    (physx::PxForceMode::Enum)type,
    auto_wake);
}

/**
 * Applies a torque defined in global coordinates to the rigid body.
 */
INLINE void PhysRigidBodyNode::
add_torque(const LVector3 &global_torque, ForceType type, bool auto_wake) {
  get_rigid_body()->addTorque(
    panda_ang_to_physx(global_torque),
    (physx::PxForceMode::Enum)type,
    auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_local_force_at_local_pos(const LVector3 &local_force, const LPoint3 &local_pos,
                             ForceType type, bool auto_wake) {
  physx::PxRigidBodyExt::addLocalForceAtLocalPos(
    *get_rigid_body(),
    panda_vec_to_physx(local_force),
    panda_vec_to_physx(local_pos),
    (physx::PxForceMode::Enum)type,
    auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_local_force_at_pos(const LVector3 &local_force, const LPoint3 &global_pos,
                       ForceType type, bool auto_wake) {
  physx::PxRigidBodyExt::addLocalForceAtPos(
    *get_rigid_body(),
    panda_vec_to_physx(local_force),
    panda_vec_to_physx(global_pos),
    (physx::PxForceMode::Enum)type,
    auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_force_at_local_pos(const LVector3 &global_force, const LPoint3 &local_pos,
                       ForceType type, bool auto_wake) {
  physx::PxRigidBodyExt::addForceAtLocalPos(
    *get_rigid_body(),
    panda_vec_to_physx(global_force),
    panda_vec_to_physx(local_pos),
    (physx::PxForceMode::Enum)type,
    auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
add_force_at_pos(const LVector3 &global_force, const LPoint3 &global_pos,
                 ForceType type, bool auto_wake) {
  physx::PxRigidBodyExt::addForceAtPos(
    *get_rigid_body(),
    panda_vec_to_physx(global_force),
    panda_vec_to_physx(global_pos),
    (physx::PxForceMode::Enum)type,
    auto_wake);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
clear_force(ForceType type) {
  get_rigid_body()->clearForce((physx::PxForceMode::Enum)type);
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
clear_torque(ForceType type) {
  get_rigid_body()->clearTorque((physx::PxForceMode::Enum)type);
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
  get_rigid_body()->setMaxDepenetrationVelocity(panda_length_to_physx(bias_clamp));
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_depenetration_velocity() const {
  return physx_length_to_panda(get_rigid_body()->getMaxDepenetrationVelocity());
}

/**
 *
 */
INLINE void PhysRigidBodyNode::
set_max_contact_impulse(PN_stdfloat max) {
  get_rigid_body()->setMaxContactImpulse(panda_length_to_physx(max));
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidBodyNode::
get_max_contact_impulse() const {
  return physx_length_to_panda(get_rigid_body()->getMaxContactImpulse());
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

/**
 * Enables or disables gravity on this specific rigid body.  Can be used to
 * implement custom gravity for a particular object, regardless of the scene
 * gravity.
 */
INLINE void PhysRigidBodyNode::
set_gravity_enabled(bool flag) {
  get_rigid_body()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !flag);
}

/**
 * Returns true if scene gravity is enabled on this rigid body.
 */
INLINE bool PhysRigidBodyNode::
is_gravity_enabled() const {
  return !get_rigid_body()->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
}

/**
 * Returns the center of mass of the body.  Requires calling
 * compute_mass_properties() to actually compute the COM.
 */
INLINE const LPoint3 &PhysRigidBodyNode::
get_center_of_mass() const {
  return _center_of_mass;
}
