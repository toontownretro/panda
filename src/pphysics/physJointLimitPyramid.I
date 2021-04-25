/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitPyramid.I
 * @author brian
 * @date 2021-04-23
 */

/**
 *
 */
INLINE PhysJointLimitPyramid::
PhysJointLimitPyramid(PN_stdfloat y_min, PN_stdfloat y_max,
                      PN_stdfloat z_min, PN_stdfloat z_max,
                      PN_stdfloat contact_dist) :
  _limit(deg_2_rad(y_min), deg_2_rad(y_max), deg_2_rad(z_min), deg_2_rad(z_max), contact_dist)
{
}

/**
 *
 */
INLINE PhysJointLimitPyramid::
PhysJointLimitPyramid(PN_stdfloat y_min, PN_stdfloat y_max,
                      PN_stdfloat z_min, PN_stdfloat z_max,
                      PN_stdfloat stiffness, PN_stdfloat damping) :
  _limit(deg_2_rad(y_min), deg_2_rad(y_max), deg_2_rad(z_min), deg_2_rad(z_max), physx::PxSpring(stiffness, damping))
{
}

/**
 *
 */
INLINE PhysJointLimitPyramid::
PhysJointLimitPyramid(const physx::PxJointLimitPyramid &pxlimit) :
  _limit(pxlimit)
{
}

/**
 *
 */
INLINE void PhysJointLimitPyramid::
set_y_range(PN_stdfloat y_min, PN_stdfloat y_max) {
  _limit.yAngleMin = deg_2_rad(y_min);
  _limit.yAngleMax = deg_2_rad(y_max);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitPyramid::
get_y_min() const {
  return rad_2_deg(_limit.yAngleMin);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitPyramid::
get_y_max() const {
  return rad_2_deg(_limit.yAngleMax);
}

/**
 *
 */
INLINE void PhysJointLimitPyramid::
set_z_range(PN_stdfloat z_min, PN_stdfloat z_max) {
  _limit.zAngleMin = deg_2_rad(z_min);
  _limit.zAngleMax = deg_2_rad(z_max);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitPyramid::
get_z_min() const {
  return rad_2_deg(_limit.zAngleMin);
}

/**
 *
 */
INLINE PN_stdfloat PhysJointLimitPyramid::
get_z_max() const {
  return rad_2_deg(_limit.zAngleMax);
}

/**
 *
 */
INLINE bool PhysJointLimitPyramid::
is_valid() const {
  return _limit.isValid();
}

/**
 *
 */
INLINE const physx::PxJointLimitPyramid &PhysJointLimitPyramid::
get_limit_pyramid() const {
  return _limit;
}
