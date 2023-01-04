/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physCapsuleController.cxx
 * @author brian
 * @date 2023-01-04
 */

#include "physCapsuleController.h"
#include "physScene.h"

/**
 *
 */
PhysCapsuleController::
PhysCapsuleController(PhysScene *scene, NodePath node, PN_stdfloat radius,
                      PN_stdfloat height, PhysMaterial *mat) {
  physx::PxCapsuleControllerDesc desc;
  // Our interface specifies the height of the entire capsule, not just
  // the distance between the two hemispheres.
  desc.radius = panda_length_to_physx(radius);
  desc.height = panda_length_to_physx(height - radius * 2);
  desc.material = mat->get_material();
  desc.upDirection = panda_norm_vec_to_physx(LVector3::up());
  desc.reportCallback = PhysControllerHitCallback::get_global_ptr();
  desc.scaleCoeff = 0.9878f;
  desc.position = panda_vec_to_physx_ex(node.get_pos(NodePath()));
  _controller = (physx::PxCapsuleController *)scene->get_controller_manager()
    ->createController(desc);
  nassertv(_controller != nullptr);
  _controller->setUserData(this);
  _actor_node = new PhysRigidDynamicNode(_controller->getActor());
}

/**
 *
 */
PhysCapsuleController::
~PhysCapsuleController() {
  destroy();
}

/**
 *
 */
physx::PxController *PhysCapsuleController::
get_controller() const {
  return _controller;
}

/**
 *
 */
void PhysCapsuleController::
destroy() {
  if (_controller != nullptr) {
    _actor_node = nullptr;
    _controller->setUserData(nullptr);
    _controller->release();
    _controller = nullptr;
  }
}
