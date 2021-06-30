/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physShape.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physShape.h"
#include "physSystem.h"
#include "physPlane.h"
#include "physx_utils.h"

TypeHandle PhysShape::_type_handle;

/**
 *
 */
PhysShape::
PhysShape(PhysGeometry &geometry, PhysMaterial *material) {
  PhysSystem *sys = PhysSystem::ptr();

  _shape = sys->get_physics()->createShape(*geometry.get_geometry(),
                                           *material->get_material(), true);
  _shape->userData = this;

  // Handle PhysX's peculiar way of specifying plane geometry.
  physx::PxGeometry *px_geom = geometry.get_geometry();
  if (px_geom->getType() == physx::PxGeometryType::ePLANE) {
    const LPlane &plane = ((PhysPlane &)geometry).get_plane();
    _shape->setLocalPose(
      physx::PxTransformFromPlaneEquation(
        physx::PxPlane(plane[0], plane[1], plane[2], panda_length_to_physx(plane[3]))));
  }

  _material = material;
}

/**
 *
 */
PhysShape::
~PhysShape() {
  if (_shape != nullptr) {
    _shape->userData = nullptr;
    _shape->release();
    _shape = nullptr;
  }
}

/**
 * Initializes a PhysShape from an existing PxShape instance.
 */
PhysShape::
PhysShape(physx::PxShape *shape) {
  _shape = shape;
  _shape->userData = this;
  _shape->acquireReference();
  physx::PxMaterial *mat;
  shape->getMaterials(&mat, 1);
  _material = new PhysMaterial(mat);
}
