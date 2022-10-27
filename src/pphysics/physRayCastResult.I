/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRayCastResult.I
 * @author brian
 * @date 2021-04-15
 */

/**
 *
 */
INLINE PhysRayCastResult::
PhysRayCastResult() :
  _buffer(_hit_buffer, 256)
{
}

/**
 *
 */
INLINE PhysRigidActorNode *PhysRayCastHit::
get_actor() const {
  return (PhysRigidActorNode *)_hit->actor->userData;
}

/**
 *
 */
INLINE PhysShape *PhysRayCastHit::
get_shape() const {
  return (PhysShape *)_hit->shape->userData;
}

/**
 *
 */
INLINE PhysRayCastHit::
PhysRayCastHit(const physx::PxRaycastHit *hit) :
  _hit(hit)
{
}

/**
 *
 */
INLINE bool PhysRayCastHit::
has_position() const {
  return _hit->flags.isSet(physx::PxHitFlag::ePOSITION);
}

/**
 *
 */
INLINE LPoint3 PhysRayCastHit::
get_position() const {
  return physx_vec_to_panda(_hit->position);
}

/**
 *
 */
INLINE bool PhysRayCastHit::
has_normal() const {
  return _hit->flags.isSet(physx::PxHitFlag::eNORMAL);
}

/**
 *
 */
INLINE LVector3 PhysRayCastHit::
get_normal() const {
  return physx_norm_vec_to_panda(_hit->normal);
}

/**
 *
 */
INLINE bool PhysRayCastHit::
has_uv() const {
  return _hit->flags.isSet(physx::PxHitFlag::eUV);
}

/**
 *
 */
INLINE LPoint2 PhysRayCastHit::
get_uv() const {
  return LPoint2(_hit->u, _hit->v);
}

/**
 *
 */
INLINE bool PhysRayCastHit::
has_face_index() const {
  return _hit->flags.isSet(physx::PxHitFlag::eFACE_INDEX);
}

/**
 *
 */
INLINE size_t PhysRayCastHit::
get_face_index() const {
  return _hit->faceIndex;
}

/**
 *
 */
INLINE PN_stdfloat PhysRayCastHit::
get_distance() const {
  return physx_length_to_panda(_hit->distance);
}

/**
 *
 */
INLINE bool PhysRayCastResult::
has_block() const {
  return _buffer.hasBlock;
}

/**
 *
 */
INLINE PhysRayCastHit PhysRayCastResult::
get_block() const {
  return PhysRayCastHit(&_buffer.block);
}

/**
 *
 */
INLINE size_t PhysRayCastResult::
get_num_any_hits() const {
  return _buffer.getNbAnyHits();
}

/**
 *
 */
INLINE PhysRayCastHit PhysRayCastResult::
get_any_hit(size_t n) const {
  return PhysRayCastHit(&_buffer.getAnyHit(n));
}

/**
 *
 */
INLINE size_t PhysRayCastResult::
get_num_touches() const {
  return _buffer.getNbTouches();
}

/**
 *
 */
INLINE PhysRayCastHit PhysRayCastResult::
get_touch(size_t n) const {
  return PhysRayCastHit(&_buffer.getTouch(n));
}

/**
 *
 */
INLINE physx::PxRaycastBuffer &PhysRayCastResult::
get_buffer() {
  return _buffer;
}

/**
 *
 */
INLINE physx::PxRaycastHit *PhysRayCastResult::
get_hit_buffer() {
  return _hit_buffer;
}
