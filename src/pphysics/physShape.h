/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physShape.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSSHAPE_H
#define PHYSSHAPE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "transformState.h"
#include "physGeometry.h"
#include "physMaterial.h"

#include "physx_includes.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysShape final : public ReferenceCount {
PUBLISHED:
  PhysShape(PhysGeometry &geometry, PhysMaterial *material);
  ~PhysShape();

  INLINE void set_local_transform(const LPoint3 &pos, const LVecBase3 &hpr);
  INLINE void set_local_pos(const LPoint3 &pos);
  INLINE void set_local_hpr(const LVecBase3 &hpr);
  INLINE LPoint3 get_local_pos() const;
  INLINE LVecBase3 get_local_hpr() const;

  INLINE void set_simulation_shape(bool flag);
  INLINE bool is_simulation_shape() const;

  INLINE void set_scene_query_shape(bool flag);
  INLINE bool is_scene_query_shape() const;

  INLINE void set_trigger_shape(bool flag);
  INLINE bool is_trigger_shape() const;

public:
  INLINE physx::PxShape *get_shape() const;

private:
  physx::PxShape *_shape;
};

#include "physShape.I"

#endif // PHYSSHAPE_H
