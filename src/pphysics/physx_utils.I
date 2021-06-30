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
 * Converts a feet measurement into meters.
 */
INLINE float
feet_to_meters(float feet) {
  return feet * 0.3048f;
}

/**
 * Converts a meters measurement into feet.
 */
INLINE float
meters_to_feet(float meters) {
  return meters * 3.28084f;
}

/**
 * Converts an inches measurement into meters.
 */
INLINE float
inches_to_meters(float inches) {
  return inches * 0.0254f;
}

/**
 * Converts a meters measurement into inches.
 */
INLINE float
meters_to_inches(float meters) {
  return meters * 39.3701f;
}

/**
 * Converts a millimeters measurement into meters.
 */
INLINE float
mm_to_meters(float mm) {
  return mm * 0.001f;
}

/**
 * Converts a meters measurement into millimeters.
 */
INLINE float
meters_to_mm(float meters) {
  return meters * 1000.0f;
}

/**
 *
 */
INLINE float
cm_to_meters(float cm) {
  return cm * 0.01f;
}

/**
 *
 */
INLINE float
meters_to_cm(float meters) {
  return meters * 100.0f;
}

/**
 *
 */
INLINE float
g_to_kg(float g) {
  return g * 0.001f;
}

/**
 *
 */
INLINE float
kg_to_g(float kg) {
  return kg * 100.0f;
}

/**
 *
 */
INLINE float
mg_to_kg(float mg) {
  return mg * 1e-6;
}

/**
 *
 */
INLINE float
kg_to_mg(float kg) {
  return kg * 1000000.0f;
}

/**
 *
 */
INLINE float
lb_to_kg(float lb) {
  return lb * 0.453592f;
}

/**
 *
 */
INLINE float
kg_to_lb(float kg) {
  return kg * 2.20462f;
}

/**
 *
 */
INLINE float
oz_to_kg(float oz) {
  return oz * 0.0283495f;
}

/**
 *
 */
INLINE float
kg_to_oz(float kg) {
  return kg * 35.274f;
}

/**
 *
 */
INLINE physx::PxVec3
panda_vec_to_physx(const LVecBase3 &vec) {
  return physx::PxVec3(
    panda_length_to_physx(vec[0]),
    panda_length_to_physx(vec[1]),
    panda_length_to_physx(vec[2])
  );
}

/**
 *
 */
INLINE physx::PxExtendedVec3
panda_vec_to_physx_ex(const LVecBase3 &vec) {
  return physx::PxExtendedVec3(
    panda_length_to_physx(vec[0]),
    panda_length_to_physx(vec[1]),
    panda_length_to_physx(vec[2])
  );
}

/**
 *
 */
INLINE LVecBase3
physx_vec_to_panda(const physx::PxVec3 &vec) {
  return LVecBase3(
    physx_length_to_panda(vec.x),
    physx_length_to_panda(vec.y),
    physx_length_to_panda(vec.z)
  );
}

/**
 *
 */
INLINE LVecBase3
physx_ex_vec_to_panda(const physx::PxExtendedVec3 &vec) {
  return LVecBase3(
    physx_length_to_panda(vec.x),
    physx_length_to_panda(vec.y),
    physx_length_to_panda(vec.z)
  );
}

/**
 * Converts a PhysX vector to Panda without doing any conversion scaling,
 * typically for unit-length direction vectors.
 */
INLINE LVecBase3
physx_norm_vec_to_panda(const physx::PxVec3 &vec) {
  return LVecBase3(vec.x, vec.y, vec.z);
}

/**
 * Converts a Panda vector to PhysX without doing any conversion scaling,
 * typically for unit-length direction vectors.
 */
INLINE physx::PxVec3
panda_norm_vec_to_physx(const LVecBase3 &vec) {
  return physx::PxVec3(vec[0], vec[1], vec[2]);
}

/**
 *
 */
INLINE physx::PxQuat
panda_quat_to_physx(const LQuaternion &quat) {
  return physx::PxQuat(
    quat[1], quat[2], quat[3], quat[0]
  );
}

/**
 *
 */
INLINE LQuaternion
physx_quat_to_panda(const physx::PxQuat &quat) {
  return LQuaternion(
    quat.w, quat.x, quat.y, quat.z
  );
}

/**
 *
 */
INLINE float
panda_ang_to_physx(float ang) {
  return deg_2_rad(ang);
}

/**
 *
 */
INLINE float
physx_ang_to_panda(float ang) {
  return rad_2_deg(ang);
}

/**
 *
 */
INLINE LVecBase3
physx_ang_to_panda(const physx::PxVec3 &ang) {
  return LVecBase3(
    physx_ang_to_panda(ang.x),
    physx_ang_to_panda(ang.y),
    physx_ang_to_panda(ang.z)
  );
}

/**
 *
 */
INLINE physx::PxVec3
panda_ang_to_physx(const LVecBase3 &ang) {
  return physx::PxVec3(
    panda_ang_to_physx(ang[0]),
    panda_ang_to_physx(ang[1]),
    panda_ang_to_physx(ang[2])
  );
}

/**
 *
 */
INLINE physx::PxTransform
panda_trans_to_physx(const TransformState *trans) {
  return physx::PxTransform(
    panda_vec_to_physx(trans->get_pos()),
    panda_quat_to_physx(trans->get_norm_quat())
  );
}

/**
 *
 */
INLINE CPT(TransformState)
physx_trans_to_panda(const physx::PxTransform &trans) {
  return TransformState::make_pos_quat(
    physx_vec_to_panda(trans.p),
    physx_quat_to_panda(trans.q)
  );
}
