/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physController.h
 * @author brian
 * @date 2021-04-27
 */

#ifndef PHYSCONTROLLER_H
#define PHYSCONTROLLER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "luse.h"
#include "collideMask.h"

#include "physx_includes.h"
#include "physx_utils.h"

/**
 * Base character controller.
 */
class EXPCL_PANDA_PPHYSICS PhysController : public ReferenceCount {
PUBLISHED:
  enum ShapeType {
    ST_box,
    ST_capsule,
  };

  enum CollisionFlags {
    CF_sides = 1 << 0,
    CF_up = 1 << 1,
    CF_down = 1 << 2,
  };

  INLINE ShapeType get_shape_type() const;

  INLINE void set_position(const LPoint3 &pos);
  INLINE LPoint3 get_position() const;

  INLINE void set_foot_position(const LPoint3 &pos);
  INLINE LPoint3 get_foot_position() const;

  INLINE void set_step_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_step_offset() const;

  INLINE void set_contact_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_contact_offset() const;

  INLINE void set_up_direction(const LVector3 &dir);
  INLINE LVector3 get_up_direction() const;

  INLINE void set_slope_limit(PN_stdfloat limit);
  INLINE PN_stdfloat get_slope_limit() const;

  INLINE void resize(PN_stdfloat size);

  INLINE void set_collide_mask(CollideMask mask);
  INLINE CollideMask get_collide_mask() const;

  CollisionFlags move(double dt, const LVector3 &move_vector, PN_stdfloat min_distance);

protected:
  virtual physx::PxController *get_controller() const = 0;

  // The node that gets controlled.
  NodePath _np;

  CollideMask _group_mask;
};

#include "physController.I"

#endif // PHYSCONTROLLER_H
