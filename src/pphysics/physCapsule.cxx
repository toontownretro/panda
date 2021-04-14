/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physCapsule.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physCapsule.h"

/**
 *
 */
PhysCapsule::
PhysCapsule(PN_stdfloat radius, PN_stdfloat half_height) :
  _geom((physx::PxReal)radius, (physx::PxReal)half_height)
{
}

/**
 *
 */
physx::PxGeometry *PhysCapsule::
get_geometry() {
  return &_geom;
}
