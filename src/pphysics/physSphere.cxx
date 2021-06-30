/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSphere.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physSphere.h"

/**
 *
 */
PhysSphere::
PhysSphere(PN_stdfloat radius) :
  _geom(panda_length_to_physx(radius))
{
}

/**
 *
 */
physx::PxGeometry *PhysSphere::
get_geometry() {
  return &_geom;
}
