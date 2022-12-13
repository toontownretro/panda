/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physContactCallbackData.I
 * @author brian
 * @date 2021-04-24
 */

/**
 *
 */
INLINE PhysContactPoint::
PhysContactPoint(const physx::PxContactPairPoint &point) :
  _point(point)
{
}

/**
 *
 */
INLINE LPoint3 PhysContactPoint::
get_position() const {
  return physx_vec_to_panda(_point.position);
}

/**
 *
 */
INLINE PN_stdfloat PhysContactPoint::
get_separation() const {
  return physx_length_to_panda(_point.separation);
}

/**
 *
 */
INLINE LVector3 PhysContactPoint::
get_normal() const {
  return physx_norm_vec_to_panda(_point.normal);
}

/**
 *
 */
INLINE LVector3 PhysContactPoint::
get_impulse() const {
  return physx_vec_to_panda(_point.impulse);
}

/**
 *
 */
INLINE size_t PhysContactPoint::
get_face_index_a() const {
  return _point.internalFaceIndex0;
}

/**
 *
 */
INLINE size_t PhysContactPoint::
get_face_index_b() const {
  return _point.internalFaceIndex1;
}

/**
 *
 */
INLINE PhysShape *PhysContactPair::
get_shape_a() const {
  return _shape_a;
}

/**
 *
 */
INLINE PhysShape *PhysContactPair::
get_shape_b() const {
  return _shape_b;
}

/**
 *
 */
INLINE int PhysContactPair::
get_contact_type() const {
  return _contact_type;
}

/**
 *
 */
INLINE bool PhysContactPair::
is_contact_type(int flags) const {
  return (_contact_type & flags) != 0;
}

/**
 *
 */
INLINE PhysContactPoint PhysContactPair::
get_contact_point(size_t n) const {
  return PhysContactPoint(_contact_points[n]);
}

/**
 *
 */
INLINE size_t PhysContactPair::
get_num_contact_points() const {
  return _contact_points.size();
}

/**
 *
 */
INLINE PT(PhysRigidActorNode) PhysContactCallbackData::
get_actor_a() const {
  return _a.lock();
}

/**
 *
 */
INLINE PT(PhysRigidActorNode) PhysContactCallbackData::
get_actor_b() const {
  return _b.lock();
}

/**
 *
 */
INLINE const PhysContactPair *PhysContactCallbackData::
get_contact_pair(size_t n) const {
  return &_contact_pairs[n];
}

/**
 *
 */
INLINE size_t PhysContactCallbackData::
get_num_contact_pairs() const {
  return _contact_pairs.size();
}
