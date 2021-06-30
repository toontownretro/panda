/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physBox.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physBox.h"

/**
 *
 */
PhysBox::
PhysBox(const LVector3 &half_extents) :
  _geom(panda_vec_to_physx(half_extents))
{
}

/**
 *
 */
PhysBox::
PhysBox(PN_stdfloat hx, PN_stdfloat hy, PN_stdfloat hz) :
  _geom(panda_vec_to_physx(LVecBase3(hx, hy, hz)))
{
}

/**
 *
 */
physx::PxGeometry *PhysBox::
get_geometry() {
  return &_geom;
}
