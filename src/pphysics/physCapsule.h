/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physCapsule.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSCAPSULE_H
#define PHYSCAPSULE_H

#include "pandabase.h"
#include "numeric_types.h"
#include "physGeometry.h"

#include "physx_includes.h"

/**
 * A capsule shape.
 */
class EXPCL_PANDA_PPHYSICS PhysCapsule final : public PhysGeometry {
PUBLISHED:
  PhysCapsule(PN_stdfloat radius = 1.0f, PN_stdfloat half_height = 1.0f);
  ~PhysCapsule() = default;

  INLINE void set_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_radius() const;

  INLINE void set_half_height(PN_stdfloat half_height);
  INLINE PN_stdfloat get_half_height() const;

  INLINE bool is_valid() const;

public:
  virtual physx::PxGeometry *get_geometry() override;

private:
  physx::PxCapsuleGeometry _geom;
};

#include "physCapsule.I"

#endif // PHYSCAPSULE_H
