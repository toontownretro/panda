/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physPlane.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSPLANE_H
#define PHYSPLANE_H

#include "pandabase.h"
#include "plane.h"
#include "physGeometry.h"

#include "physx_includes.h"

/**
 * A plane shape.
 */
class EXPCL_PANDA_PPHYSICS PhysPlane final : public PhysGeometry {
PUBLISHED:
  PhysPlane(const LPlane &plane);
  ~PhysPlane() = default;

  INLINE void set_plane(const LPlane &plane);
  INLINE const LPlane &get_plane() const;

  INLINE bool is_valid() const;

public:
  virtual physx::PxGeometry *get_geometry() override;

private:
  physx::PxPlaneGeometry _geom;
  LPlane _plane;
};

#include "physPlane.I"

#endif // PHYSPLANE_H
