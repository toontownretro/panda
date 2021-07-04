/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitAngularPair.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PhysJointLimitAngularPair::
PhysJointLimitAngularPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit,
                          PN_stdfloat contact_dist) :
  _limit(deg_2_rad(lower_limit), deg_2_rad(upper_limit),
         contact_dist != -1.0f ? deg_2_rad(contact_dist) : contact_dist)
{
}

/**
 *
 */
INLINE PhysJointLimitAngularPair::
PhysJointLimitAngularPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit,
                          PN_stdfloat stiffness, PN_stdfloat damping) :
  _limit(deg_2_rad(lower_limit), deg_2_rad(upper_limit), physx::PxSpring(stiffness, damping))
{
}

/**
 *
 */
INLINE PhysJointLimitAngularPair::
PhysJointLimitAngularPair(const physx::PxJointAngularLimitPair &pxlimit) :
  _limit(pxlimit)
{
}

/**
 *
 */
INLINE void PhysJointLimitAngularPair::
set_upper_limit(PN_stdfloat angle) {
  _limit.upper = deg_2_rad(angle);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitAngularPair::
get_upper_limit() const {
  return rad_2_deg(_limit.upper);
}

/**
 *
 */
INLINE void PhysJointLimitAngularPair::
set_lower_limit(PN_stdfloat angle) {
  _limit.lower = deg_2_rad(angle);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitAngularPair::
get_lower_limit() const {
  return rad_2_deg(_limit.lower);
}

/**
 *
 */
INLINE bool PhysJointLimitAngularPair::
is_valid() const {
  return _limit.isValid();
}

/**
 *
 */
INLINE const physx::PxJointAngularLimitPair &PhysJointLimitAngularPair::
get_limit_pair() const {
  return _limit;
}
