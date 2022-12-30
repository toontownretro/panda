/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelUser.I
 * @author brian
 * @date 2022-12-29
 */

/**
 *
 */
INLINE void AnimChannelUser::
set_joint_position(int i, const LPoint3 &pos) {
  _pose_data._pose[i / SIMDFloatVector::num_columns].pos.
    set_lvec(i % SIMDFloatVector::num_columns, pos);
}

/**
 *
 */
INLINE LPoint3 AnimChannelUser::
get_joint_position(int i) const {
  return _pose_data._pose[i / SIMDFloatVector::num_columns].pos.
    get_lvec(i % SIMDFloatVector::num_columns);
}

/**
 *
 */
INLINE void AnimChannelUser::
set_joint_scale(int i, const LVecBase3 &scale) {
  _pose_data._pose[i / SIMDFloatVector::num_columns].scale.
    set_lvec(i % SIMDFloatVector::num_columns, scale);
}

/**
 *
 */
INLINE LVecBase3 AnimChannelUser::
get_joint_scale(int i) const {
  return _pose_data._pose[i / SIMDFloatVector::num_columns].scale.
    get_lvec(i % SIMDFloatVector::num_columns);
}

/**
 *
 */
INLINE void AnimChannelUser::
set_joint_shear(int i, const LVecBase3 &shear) {
  _pose_data._pose[i / SIMDFloatVector::num_columns].shear.
    set_lvec(i % SIMDFloatVector::num_columns, shear);
}

/**
 *
 */
INLINE LVecBase3 AnimChannelUser::
get_joint_shear(int i) const {
  return _pose_data._pose[i / SIMDFloatVector::num_columns].shear.
    get_lvec(i % SIMDFloatVector::num_columns);
}

/**
 *
 */
INLINE void AnimChannelUser::
set_joint_quat(int i, const LQuaternion &quat) {
  _pose_data._pose[i / SIMDFloatVector::num_columns].quat.
    set_lquat(i % SIMDFloatVector::num_columns, quat);
}

/**
 *
 */
INLINE LQuaternion AnimChannelUser::
get_joint_quat(int i) const {
  return _pose_data._pose[i / SIMDFloatVector::num_columns].quat.
    get_lquat(i % SIMDFloatVector::num_columns);
}

/**
 *
 */
INLINE void AnimChannelUser::
set_joint_hpr(int i, const LVecBase3 &hpr) {
  LQuaternion quat;
  quat.set_hpr(hpr);
  set_joint_quat(i, quat);
}

/**
 *
 */
INLINE LVecBase3 AnimChannelUser::
get_joint_hpr(int i) const {
  return get_joint_quat(i).get_hpr();
}

/**
 *
 */
INLINE void AnimChannelUser::
set_slider(int i, PN_stdfloat value) {
  _pose_data._sliders[i / SIMDFloatVector::num_columns][i % SIMDFloatVector::num_columns] = value;
}

/**
 *
 */
INLINE PN_stdfloat AnimChannelUser::
get_slider(int i) const {
  return _pose_data._sliders[i / SIMDFloatVector::num_columns][i % SIMDFloatVector::num_columns];
}
