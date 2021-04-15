/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physMaterial.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physMaterial.h"
#include "physSystem.h"

/**
 *
 */
PhysMaterial::
PhysMaterial(PN_stdfloat static_friction, PN_stdfloat dynamic_friction,
             PN_stdfloat restitution)
{
  PhysSystem *sys = PhysSystem::ptr();
  _material = sys->get_physics()->createMaterial(
    (physx::PxReal)static_friction, (physx::PxReal)dynamic_friction,
    (physx::PxReal)restitution);
  _material->userData = this;
}

/**
 *
 */
PhysMaterial::
~PhysMaterial() {
  if (_material != nullptr) {
    _material->release();
    _material = nullptr;
  }
}
