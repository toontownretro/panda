/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physx_utils.I
 * @author brian
 * @date 2021-04-21
 */

/**
 *
 */
INLINE LVecBase3
PxVec3_to_Vec3(const physx::PxVec3 &vec) {
  return LVecBase3(vec.x, vec.y, vec.z);
}

/**
 *
 */
INLINE physx::PxVec3
Vec3_to_PxVec3(const LVecBase3 &vec) {
  return physx::PxVec3(vec[0], vec[1], vec[2]);
}

/**
 *
 */
INLINE LQuaternion
PxQuat_to_Quat(const physx::PxQuat &quat) {
  return LQuaternion(quat.w, quat.x, quat.y, quat.z);
}

/**
 *
 */
INLINE physx::PxQuat
Quat_to_PxQuat(const LQuaternion &quat) {
  return physx::PxQuat(quat[1], quat[2], quat[3], quat[0]);
}

/**
 *
 */
INLINE CPT(TransformState)
PxTransform_to_TransformState(const physx::PxTransform &transform) {
  return TransformState::make_pos_quat(
    PxVec3_to_Vec3(transform.p), PxQuat_to_Quat(transform.q));
}

/**
 *
 */
INLINE physx::PxTransform
TransformState_to_PxTransform(const TransformState *state) {
  return physx::PxTransform(
    Vec3_to_PxVec3(state->get_pos()), Quat_to_PxQuat(state->get_quat()));
}
