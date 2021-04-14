/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physGeometry.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSGEOMETRY_H
#define PHYSGEOMETRY_H

#include "pandabase.h"
#include "geometry/PxGeometry.h"

/**
 * Base physics geometry class.
 */
class EXPCL_PANDA_PPHYSICS PhysGeometry {
public:
  virtual physx::PxGeometry *get_geometry() = 0;
};

#endif // PHYSGEOMETRY_H
