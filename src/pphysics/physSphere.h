/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSphere.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSSPHERE_H
#define PHYSSPHERE_H

#include "pandabase.h"
#include "numeric_types.h"
#include "physGeometry.h"

#include "geometry/PxSphereGeometry.h"

/**
 * A sphere with a radius.
 */
class EXPCL_PANDA_PPHYSICS PhysSphere final : public PhysGeometry {
PUBLISHED:
  PhysSphere(PN_stdfloat radius = 1.0f);

  INLINE void set_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_radius() const;

  INLINE bool is_valid() const;

public:
  virtual physx::PxGeometry *get_geometry() override;

private:
  physx::PxSphereGeometry _geom;
};

#include "physSphere.I"

#endif // PHYSSPHERE_H
