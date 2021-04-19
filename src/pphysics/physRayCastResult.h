/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRayCastResult.h
 * @author brian
 * @date 2021-04-15
 */

#ifndef PHYSRAYCASTRESULT_H
#define PHYSRAYCASTRESULT_H

#include "pandabase.h"
#include "luse.h"
#include "physRigidActorNode.h"
#include "physShape.h"

#include "physx_includes.h"

/**
 * Contains the resulting information of a single ray-cast intersection.
 */
class EXPCL_PANDA_PPHYSICS PhysRayCastHit {
public:
  INLINE PhysRayCastHit(const physx::PxRaycastHit *hit);

PUBLISHED:
  ~PhysRayCastHit() = default;

  INLINE PhysRigidActorNode *get_actor() const;
  INLINE PhysShape *get_shape() const;

  INLINE bool has_position() const;
  INLINE LPoint3 get_position() const;

  INLINE bool has_normal() const;
  INLINE LVector3 get_normal() const;

  INLINE bool has_uv() const;
  INLINE LPoint2 get_uv() const;

  INLINE bool has_face_index() const;
  INLINE size_t get_face_index() const;

private:
  const physx::PxRaycastHit *_hit;
};

/**
 * Contains the resulting information of a single ray-cast query.
 */
class EXPCL_PANDA_PPHYSICS PhysRayCastResult {
PUBLISHED:
  INLINE PhysRayCastResult();
  PhysRayCastResult(const PhysRayCastResult &copy) = delete;

  ~PhysRayCastResult() = default;

  void operator = (const PhysRayCastResult &copy) = delete;

  INLINE bool has_block() const;
  INLINE PhysRayCastHit get_block() const;

  INLINE size_t get_num_any_hits() const;
  INLINE PhysRayCastHit get_any_hit(size_t n) const;

  INLINE size_t get_num_touches() const;
  INLINE PhysRayCastHit get_touch(size_t n) const;

public:
  INLINE physx::PxRaycastBuffer &get_buffer();
  INLINE physx::PxRaycastHit *get_hit_buffer();

private:
  physx::PxRaycastBuffer _buffer;
  physx::PxRaycastHit _hit_buffer[256];
};

#include "physRayCastResult.I"

#endif // PHYSRAYCASTRESULT_H
