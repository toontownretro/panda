/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSweepResult.h
 * @author brian
 * @date 2021-06-21
 */

#ifndef PHYSSWEEPRESULT_H
#define PHYSSWEEPRESULT_H

#include "pandabase.h"
#include "luse.h"
#include "physRigidActorNode.h"
#include "physShape.h"

#include "physx_includes.h"

/**
 * Contains the resulting information of a single sweep intersection.
 */
class EXPCL_PANDA_PPHYSICS PhysSweepHit {
public:
  INLINE PhysSweepHit(const physx::PxSweepHit *hit);

PUBLISHED:
  virtual ~PhysSweepHit() = default;

  INLINE PhysRigidActorNode *get_actor() const;
  INLINE PhysShape *get_shape() const;

  INLINE bool has_position() const;
  INLINE LPoint3 get_position() const;

  INLINE bool has_normal() const;
  INLINE LVector3 get_normal() const;

  INLINE bool has_face_index() const;
  INLINE size_t get_face_index() const;

  INLINE PN_stdfloat get_distance() const;

  PhysMaterial *get_material() const;

private:
  const physx::PxSweepHit *_hit;
};

/**
 * Contains the resulting information of a single sweep query.
 */
class EXPCL_PANDA_PPHYSICS PhysSweepResult {
PUBLISHED:
  INLINE PhysSweepResult();
  PhysSweepResult(const PhysSweepResult &copy) = delete;

  virtual ~PhysSweepResult() = default;

  void operator = (const PhysSweepResult &copy) = delete;

  INLINE bool has_block() const;
  INLINE PhysSweepHit get_block() const;

  INLINE size_t get_num_any_hits() const;
  INLINE PhysSweepHit get_any_hit(size_t n) const;

  INLINE size_t get_num_touches() const;
  INLINE PhysSweepHit get_touch(size_t n) const;

public:
  INLINE physx::PxSweepBuffer &get_buffer();
  INLINE physx::PxSweepHit *get_hit_buffer();

private:
  physx::PxSweepBuffer _buffer;
  physx::PxSweepHit _hit_buffer[256];
};

#include "physSweepResult.I"

#endif // PHYSSWEEPRESULT_H
