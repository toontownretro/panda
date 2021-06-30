/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSweepResult.I
 * @author brian
 * @date 2021-06-21
 */

/**
 *
 */
INLINE PhysSweepResult::
PhysSweepResult() :
  _buffer(_hit_buffer, 256)
{
}

/**
 *
 */
INLINE PhysRigidActorNode *PhysSweepHit::
get_actor() const {
  return (PhysRigidActorNode *)_hit->actor->userData;
}

/**
 *
 */
INLINE PhysShape *PhysSweepHit::
get_shape() const {
  return (PhysShape *)_hit->shape->userData;
}

/**
 *
 */
INLINE PhysSweepHit::
PhysSweepHit(const physx::PxSweepHit *hit) :
  _hit(hit)
{
}

/**
 *
 */
INLINE bool PhysSweepHit::
has_position() const {
  return _hit->flags.isSet(physx::PxHitFlag::ePOSITION);
}

/**
 *
 */
INLINE LPoint3 PhysSweepHit::
get_position() const {
  return physx_vec_to_panda(_hit->position);
}

/**
 *
 */
INLINE bool PhysSweepHit::
has_normal() const {
  return _hit->flags.isSet(physx::PxHitFlag::eNORMAL);
}

/**
 *
 */
INLINE LVector3 PhysSweepHit::
get_normal() const {
  return physx_norm_vec_to_panda(_hit->normal);
}

/**
 *
 */
INLINE bool PhysSweepHit::
has_face_index() const {
  return _hit->flags.isSet(physx::PxHitFlag::eFACE_INDEX);
}

/**
 *
 */
INLINE size_t PhysSweepHit::
get_face_index() const {
  return _hit->faceIndex;
}

/**
 *
 */
INLINE bool PhysSweepResult::
has_block() const {
  return _buffer.hasBlock;
}

/**
 *
 */
INLINE PhysSweepHit PhysSweepResult::
get_block() const {
  return PhysSweepHit(&_buffer.block);
}

/**
 *
 */
INLINE size_t PhysSweepResult::
get_num_any_hits() const {
  return _buffer.getNbAnyHits();
}

/**
 *
 */
INLINE PhysSweepHit PhysSweepResult::
get_any_hit(size_t n) const {
  return PhysSweepHit(&_buffer.getAnyHit(n));
}

/**
 *
 */
INLINE size_t PhysSweepResult::
get_num_touches() const {
  return _buffer.getNbTouches();
}

/**
 *
 */
INLINE PhysSweepHit PhysSweepResult::
get_touch(size_t n) const {
  return PhysSweepHit(&_buffer.getTouch(n));
}

/**
 *
 */
INLINE physx::PxSweepBuffer &PhysSweepResult::
get_buffer() {
  return _buffer;
}

/**
 *
 */
INLINE physx::PxSweepHit *PhysSweepResult::
get_hit_buffer() {
  return _hit_buffer;
}
