/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physShape.I
 * @author brian
 * @date 2021-04-14
 */

/**
 *
 */
INLINE void PhysShape::
set_local_transform(const LPoint3 &pos, const LVecBase3 &hpr) {
  nassertv(_shape != nullptr);

  LQuaternion quat;
  quat.set_hpr(hpr);

  physx::PxTransform trans = physx::PxTransform(
    panda_vec_to_physx(pos),
    panda_quat_to_physx(quat));

  _shape->setLocalPose(trans);
}

/**
 * Modifies just the position of the shape.
 */
INLINE void PhysShape::
set_local_pos(const LPoint3 &pos) {
  nassertv(_shape != nullptr);

  physx::PxTransform trans = _shape->getLocalPose();
  trans.p = panda_vec_to_physx(pos);
  _shape->setLocalPose(trans);
}

/**
 * Modifies just the orientation of the shape.
 */
INLINE void PhysShape::
set_local_hpr(const LVecBase3 &hpr) {
  nassertv(_shape != nullptr);

  LQuaternion quat;
  quat.set_hpr(hpr);

  physx::PxTransform trans = _shape->getLocalPose();
  trans.q = panda_quat_to_physx(quat);
  _shape->setLocalPose(trans);
}

/**
 *
 */
INLINE LPoint3 PhysShape::
get_local_pos() const {
  nassertr(_shape != nullptr, LPoint3());

  physx::PxTransform trans = _shape->getLocalPose();
  return physx_vec_to_panda(trans.p);
}

/**
 *
 */
INLINE LVecBase3 PhysShape::
get_local_hpr() const {
  nassertr(_shape != nullptr, LVecBase3());

  physx::PxTransform trans = _shape->getLocalPose();
  LQuaternion quat = physx_quat_to_panda(trans.q);
  return quat.get_hpr();
}

/**
 *
 */
INLINE void PhysShape::
set_simulation_shape(bool flag) {
  _shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, flag);
}

/**
 *
 */
INLINE bool PhysShape::
is_simulation_shape() const {
  return _shape->getFlags().isSet(physx::PxShapeFlag::eSIMULATION_SHAPE);
}

/**
 *
 */
INLINE void PhysShape::
set_scene_query_shape(bool flag) {
  _shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, flag);
}

/**
 *
 */
INLINE bool PhysShape::
is_scene_query_shape() const {
  return _shape->getFlags().isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
}

/**
 *
 */
INLINE void PhysShape::
set_trigger_shape(bool flag) {
  _shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, flag);
}

/**
 *
 */
INLINE bool PhysShape::
is_trigger_shape() const {
  return _shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
}

/**
 * Replaces the shape's current material.
 */
INLINE void PhysShape::
set_material(PhysMaterial *mat) {
  _material = mat;
  physx::PxMaterial *pxmat = mat->get_material();
  _shape->setMaterials(&pxmat, 1);
}

/**
 * Returns the material applied to the shape.
 */
INLINE PhysMaterial *PhysShape::
get_material() const {
  return _material;
}

/**
 *
 */
INLINE physx::PxShape *PhysShape::
get_shape() const {
  return _shape;
}
