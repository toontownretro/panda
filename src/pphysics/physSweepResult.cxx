/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSweepResult.cxx
 * @author brian
 * @date 2021-06-21
 */

#include "physSweepResult.h"
#include "physx_utils.h"

/**
 *
 */
PhysMaterial *PhysSweepHit::
get_material() const {
  return phys_material_from_shape_and_face_index(get_shape(), get_face_index());
}
