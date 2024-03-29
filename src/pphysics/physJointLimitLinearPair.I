/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitLinearPair.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PhysJointLimitLinearPair::
PhysJointLimitLinearPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit, PN_stdfloat contact_dist) :
  _limit(PhysSystem::ptr()->get_scale(), panda_length_to_physx(lower_limit),
         panda_length_to_physx(upper_limit), contact_dist != -1.0f ? panda_length_to_physx(contact_dist) : contact_dist)
{
}

/**
 *
 */
INLINE PhysJointLimitLinearPair::
PhysJointLimitLinearPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit,
                         PN_stdfloat stiffness, PN_stdfloat damping) :
  _limit(panda_length_to_physx(lower_limit), panda_length_to_physx(upper_limit), physx::PxSpring(stiffness, damping))
{
}

/**
 *
 */
INLINE PhysJointLimitLinearPair::
PhysJointLimitLinearPair(const physx::PxJointLinearLimitPair &pxlimit) :
  _limit(pxlimit)
{
}

/**
 *
 */
INLINE void PhysJointLimitLinearPair::
set_upper_limit(PN_stdfloat limit) {
  _limit.upper = panda_length_to_physx(limit);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitLinearPair::
get_upper_limit() const {
  return physx_length_to_panda(_limit.upper);
}

/**
 *
 */
INLINE void PhysJointLimitLinearPair::
set_lower_limit(PN_stdfloat limit) {
  _limit.lower = panda_length_to_physx(limit);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitLinearPair::
get_lower_limit() const {
  return physx_length_to_panda(_limit.lower);
}

/**
 *
 */
INLINE bool PhysJointLimitLinearPair::
is_valid() const {
  return _limit.isValid();
}

/**
 *
 */
INLINE const physx::PxJointLinearLimitPair &PhysJointLimitLinearPair::
get_limit_pair() const {
  return _limit;
}
