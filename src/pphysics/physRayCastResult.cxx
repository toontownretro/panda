/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRayCastResult.cxx
 * @author brian
 * @date 2021-04-15
 */

#include "physRayCastResult.h"
#include "physx_utils.h"

/**
 * Returns the PhysMaterial instance hit by the ray, or nullptr if there
 * is no valid material.
 */
PhysMaterial *PhysRayCastHit::
get_material() const {
  physx::PxShape *pxshape = _hit->shape;
  if (pxshape == nullptr) {
    // Somehow we hit no shape.
    return nullptr;
  }

  PhysShape *shape = (PhysShape *)pxshape->userData;

  return phys_material_from_shape_and_face_index(shape, get_face_index());
}
