/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimit.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE void PhysJointLimit::
set_restitution(PN_stdfloat value) {
  get_params().restitution = value;
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimit::
get_restitution() const {
  return get_params().restitution;
}

/**
 *
 */
INLINE void PhysJointLimit::
set_bounce_threshold(PN_stdfloat threshold) {
  get_params().bounceThreshold = threshold;
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimit::
get_bounce_threshold() const {
  return get_params().bounceThreshold;
}

/**
 *
 */
INLINE void PhysJointLimit::
set_stiffness(PN_stdfloat stiff) {
  get_params().stiffness = stiff;
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimit::
get_stiffness() const {
  return get_params().stiffness;
}

/**
 *
 */
INLINE void PhysJointLimit::
set_damping(PN_stdfloat damp) {
  get_params().damping = damp;
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimit::
get_damping() const {
  return get_params().damping;
}

/**
 *
 */
INLINE void PhysJointLimit::
set_contact_distance(PN_stdfloat dist) {
  get_params().contactDistance = dist;
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimit::
get_contact_distance() const {
  return get_params().contactDistance;
}

/**
 *
 */
INLINE bool PhysJointLimit::
is_valid() const {
  return get_params().isValid();
}

/**
 *
 */
INLINE bool PhysJointLimit::
is_soft() const {
  return get_params().isSoft();
}

/**
 *
 */
INLINE const physx::PxJointLimitParameters &PhysJointLimit::
get_params() const {
  return ((PhysJointLimit *)this)->get_params();
}
