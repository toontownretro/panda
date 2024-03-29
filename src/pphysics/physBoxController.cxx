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
  desc.halfSideExtent = panda_length_to_physx(half_extents[0]);
  desc.halfForwardExtent = panda_length_to_physx(half_extents[1]);
  desc.halfHeight = panda_length_to_physx(half_extents[2]);
  desc.material = material->get_material();
  desc.upDirection = panda_norm_vec_to_physx(LVector3::up());
  desc.reportCallback = PhysControllerHitCallback::get_global_ptr();
  desc.scaleCoeff = 0.9878f;
  desc.position = panda_vec_to_physx_ex(node.get_pos(NodePath()));
  _controller = (physx::PxBoxController *)scene->get_controller_manager()
    ->createController(desc);
  nassertv(_controller != nullptr);
  _controller->setUserData(this);
  _actor_node = new PhysRigidDynamicNode(_controller->getActor());
}

/**
 *
 */
PhysBoxController::
~PhysBoxController() {
  destroy();
}

/**
 *
 */
physx::PxController *PhysBoxController::
get_controller() const {
  return _controller;
}

/**
 *
 */
void PhysBoxController::
destroy() {
  if (_actor_node != nullptr) {
    // Manually remove the actor associated with the character
    // from the physics scene.  For some reason, it does not appear
    // that this is done automatically when the controller is
    // released.
    physx::PxRigidActor *actor = _actor_node->get_rigid_actor();
    if (actor != nullptr) {
      physx::PxScene *scene = actor->getScene();
      if (scene != nullptr) {
        scene->removeActor(*actor);
      }
    }
    _actor_node = nullptr;
  }
  if (_controller != nullptr) {
    _controller->setUserData(nullptr);
    _controller->release();
    _controller = nullptr;
  }
}
