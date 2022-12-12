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
  _materials.clear();
  _phys_materials.clear();
  _phys_materials.push_back(mat);
  _materials.push_back(mat->get_material());
  _shape->setMaterials(_materials.data(), 1);
}

/**
 * Appends a new material to the shape.
 */
INLINE void PhysShape::
add_material(PhysMaterial *mat) {
  _phys_materials.push_back(mat);
  _materials.push_back(mat->get_material());
  _shape->setMaterials(_materials.data(), _materials.size());
}

/**
 * Returns the nth material applied to the shape.
 */
INLINE PhysMaterial *PhysShape::
get_material(int n) const {
  nassertr(n >= 0 && n < _phys_materials.size(), nullptr);
  return _phys_materials[n];
}

/**
 * Sets the "from" collide mask of the shape.  This represents the collision
 * groups that the shape is assigned to.  During simulation, the "from" mask
 * is AND'd with the "into" mask of other shapes to determine if collisions
 * should occur between two shapes.
 */
INLINE void PhysShape::
set_from_collide_mask(CollideMask mask) {
  // Set it directly on the PxShape's filter data.
  // We don't store anything on our shape instance itself.
  physx::PxFilterData data = _shape->getSimulationFilterData();
  physx::PxFilterData qdata = _shape->getQueryFilterData();
  data.word1 = mask.get_word();
  qdata.word0 = mask.get_word();
  _shape->setSimulationFilterData(data);
  _shape->setQueryFilterData(qdata);
}

/**
 * Returns the "from" collide mask of the shape.  This represents the
 * collision groups that the shape is assigned to.
 */
INLINE CollideMask PhysShape::
get_from_collide_mask() const {
  return _shape->getSimulationFilterData().word1;
}

/**
 * Sets the "into" collide mask of the shape.  This represents the set of
 * collision groups that the shape should collide with.
 */
INLINE void PhysShape::
set_into_collide_mask(CollideMask mask) {
  physx::PxFilterData fdata;
  fdata = _shape->getSimulationFilterData();
  fdata.word2 = mask.get_word();
  _shape->setSimulationFilterData(fdata);
}

/**
 * Returns the "into" collide mask of the shape.  This represents the set of
 * collision groups that the shape should collide with.
 */
INLINE CollideMask PhysShape::
get_into_collide_mask() const {
  return _shape->getSimulationFilterData().word2;
}

/**
 *
 */
INLINE physx::PxShape *PhysShape::
get_shape() const {
  return _shape;
}
