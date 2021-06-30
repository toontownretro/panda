/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJoint.I
 * @author brian
 * @date 2021-04-21
 */

/**
 * Sets the two actor nodes constrained by this joint.
 */
INLINE void PhysJoint::
set_actors(PhysRigidActorNode *a, PhysRigidActorNode *b) {
  get_joint()->setActors(a->get_rigid_actor(), b->get_rigid_actor());
  _a = a;
  _b = b;
}

/**
 * Returns the two actor nodes constrained by this joint.
 */
INLINE PhysRigidActorNode *PhysJoint::
get_actor_a() const {
  return _a;
}

/**
 * Returns the second actor associated with the joint.
 */
INLINE PhysRigidActorNode *PhysJoint::
get_actor_b() const {
  return _b;
}

/**
 * Sets the local transform of this joint relative to actor A.
 */
INLINE void PhysJoint::
set_frame_a(const TransformState *transform) {
  get_joint()->setLocalPose(physx::PxJointActorIndex::eACTOR0,
                            panda_trans_to_physx(transform));
}

/**
 * Returns the local transform of this joint relative to actor A.
 */
INLINE CPT(TransformState) PhysJoint::
get_frame_a() const {
  return physx_trans_to_panda(
    get_joint()->getLocalPose(physx::PxJointActorIndex::eACTOR0));
}

/**
 * Sets the local transform of this joint relative to actor B.
 */
INLINE void PhysJoint::
set_frame_b(const TransformState *transform) {
  get_joint()->setLocalPose(physx::PxJointActorIndex::eACTOR1,
                            panda_trans_to_physx(transform));
}

/**
 * Returns the local transform of this joint relative to actor B.
 */
INLINE CPT(TransformState) PhysJoint::
get_frame_b() const {
  return physx_trans_to_panda(
    get_joint()->getLocalPose(physx::PxJointActorIndex::eACTOR1));
}

/**
 * Returns the relative transform of the joint.
 */
INLINE CPT(TransformState) PhysJoint::
get_relative_transform() const {
  return physx_trans_to_panda(get_joint()->getRelativeTransform());
}

/**
 *
 */
INLINE LVector3 PhysJoint::
get_relative_linear_velocity() const {
  return physx_vec_to_panda(get_joint()->getRelativeLinearVelocity());
}

/**
 *
 */
INLINE LVector3 PhysJoint::
get_relative_angular_velocity() const {
  return physx_ang_to_panda(get_joint()->getRelativeAngularVelocity());
}

/**
 *
 */
INLINE void PhysJoint::
set_break_force(PN_stdfloat force, PN_stdfloat torque) {
  get_joint()->setBreakForce(panda_length_to_physx(force),
                             panda_length_to_physx(torque));
}

/**
 *
 */
INLINE void PhysJoint::
get_break_force(PN_stdfloat &force, PN_stdfloat &torque) const {
  physx::PxReal pforce, ptorque;
  get_joint()->getBreakForce(pforce, ptorque);

  force = physx_length_to_panda(pforce);
  torque = physx_length_to_panda(ptorque);
}

/**
 *
 */
INLINE void PhysJoint::
set_inv_mass_scale_a(PN_stdfloat scale) {
  get_joint()->setInvMassScale0(scale);
}

/**
 *
 */
INLINE PN_stdfloat PhysJoint::
get_inv_mass_scale_a() const {
  return get_joint()->getInvMassScale0();
}

/**
 *
 */
INLINE void PhysJoint::
set_inv_mass_scale_b(PN_stdfloat scale) {
  get_joint()->setInvMassScale1(scale);
}

/**
 *
 */
INLINE PN_stdfloat PhysJoint::
get_inv_mass_scale_b() const {
  return get_joint()->getInvMassScale1();
}

/**
 *
 */
INLINE void PhysJoint::
set_inv_inertia_scale_a(PN_stdfloat scale) {
  get_joint()->setInvInertiaScale0(scale);
}

/**
 *
 */
INLINE PN_stdfloat PhysJoint::
get_inv_inertia_scale_a() const {
  return get_joint()->getInvInertiaScale0();
}

/**
 *
 */
INLINE void PhysJoint::
set_inv_inertia_scale_b(PN_stdfloat scale) {
  get_joint()->setInvInertiaScale1(scale);
}

/**
 *
 */
INLINE PN_stdfloat PhysJoint::
get_inv_inertia_scale_b() const {
  return get_joint()->getInvInertiaScale1();
}

/**
 *
 */
INLINE void PhysJoint::
set_projection_enabled(bool flag) {
  get_joint()->setConstraintFlag(physx::PxConstraintFlag::ePROJECTION, flag);
}

/**
 *
 */
INLINE bool PhysJoint::
get_projection_enabled() const {
  return get_joint()->getConstraintFlags().isSet(physx::PxConstraintFlag::ePROJECTION);
}

/**
 *
 */
INLINE void PhysJoint::
set_collision_enabled(bool flag) {
  get_joint()->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, flag);
}

/**
 *
 */
INLINE bool PhysJoint::
get_collision_enabled() const {
  return get_joint()->getConstraintFlags().isSet(physx::PxConstraintFlag::eCOLLISION_ENABLED);
}

/**
 * Returns true if the joint is broken, false otherwise.
 */
INLINE bool PhysJoint::
is_broken() const {
  return get_joint()->getConstraintFlags().isSet(physx::PxConstraintFlag::eBROKEN);
}

/**
 * Sets the callback to be executed when the joint breaks.
 */
INLINE void PhysJoint::
set_break_callback(CallbackObject *callback) {
  _break_callback = callback;
}

/**
 * Returns the break callback.
 */
INLINE CallbackObject *PhysJoint::
get_break_callback() const {
  return _break_callback;
}
