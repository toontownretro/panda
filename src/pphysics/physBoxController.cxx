/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physBoxController.cxx
 * @author brian
 * @date 2021-04-28
 */

#include "physBoxController.h"
#include "physScene.h"
#include "physMaterial.h"

/**
 *
 */
PhysBoxController::
PhysBoxController(PhysScene *scene, NodePath node, const LVector3 &half_extents,
                  PhysMaterial *material) {
  _np = node;
  physx::PxBoxControllerDesc desc;
  desc.halfSideExtent = half_extents[0];
  desc.halfForwardExtent = half_extents[1];
  desc.halfHeight = half_extents[2];
  desc.material = material->get_material();
  desc.upDirection = Vec3_to_PxVec3(LVector3::up());
  desc.reportCallback = PhysControllerHitCallback::get_global_ptr();
  _controller = (physx::PxBoxController *)scene->get_controller_manager()
    ->createController(desc);
  nassertv(_controller != nullptr);
  _controller->setUserData(this);
  update_shape_filter_data();
}

/**
 *
 */
PhysBoxController::
~PhysBoxController() {
  if (_controller != nullptr) {
    _controller->release();
    _controller = nullptr;
  }
}

/**
 *
 */
physx::PxController *PhysBoxController::
get_controller() const {
  return _controller;
}
