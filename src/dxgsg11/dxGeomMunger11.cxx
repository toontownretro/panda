/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGeomMunger11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxGeomMunger11.h"

TypeHandle DXGeomMunger11::_type_handle;

/**
 *
 */
DXGeomMunger11::
DXGeomMunger11(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 4, NT_uint8, C_color)
{
}
