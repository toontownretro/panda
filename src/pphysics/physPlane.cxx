/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physPlane.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physPlane.h"

PhysPlane::
PhysPlane(const LPlane &plane) :
  _plane(plane)
{
}

/**
 *
 */
physx::PxGeometry *PhysPlane::
get_geometry() {
  return &_geom;
}
