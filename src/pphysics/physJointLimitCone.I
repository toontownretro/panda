/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitCone.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PhysJointLimitCone::
PhysJointLimitCone(const physx::PxJointLimitCone &pxlimit) :
  _limit(pxlimit)
{
}

/**
 *
 */
INLINE void PhysJointLimitCone::
set_y_limit_angle(PN_stdfloat angle) {
  _limit.yAngle = deg_2_rad(angle);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitCone::
get_y_limit_angle() const {
  return rad_2_deg(_limit.yAngle);
}

/**
 *
 */
INLINE void PhysJointLimitCone::
set_z_limit_angle(PN_stdfloat angle) {
  _limit.zAngle = deg_2_rad(angle);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitCone::
get_z_limit_angle() const {
  return rad_2_deg(_limit.zAngle);
}

/**
 *
 */
INLINE bool PhysJointLimitCone::
is_valid() const {
  return _limit.isValid();
}

/**
 *
 */
INLINE const physx::PxJointLimitCone &PhysJointLimitCone::
get_limit_cone() const {
  return _limit;
}
