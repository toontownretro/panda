/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidDynamicNode.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physRigidDynamicNode.h"
#include "nodePath.h"
#include "physSystem.h"
#include "physShape.h"

#include "physx_includes.h"

TypeHandle PhysRigidDynamicNode::_type_handle;

/**
 *
 */
PhysRigidDynamicNode::
PhysRigidDynamicNode(const std::string &name) :
  PhysRigidBodyNode(name)
{
  PhysSystem *sys = PhysSystem::ptr();
  _rigid_dynamic = sys->get_physics()->createRigidDynamic(
    physx::PxTransform(physx::PxVec3(0, 0, 0)));
  _rigid_dynamic->userData = this;
  _self_created = true;
}

/**
 *
 */
PhysRigidDynamicNode::
~PhysRigidDynamicNode() {
  if (_rigid_dynamic != nullptr) {

    _rigid_dynamic->userData = nullptr;

    if (_self_created) {
      // Not sure if this is done automatically by PhysX, so I'm doing it just
      // to be safe.
      if (_rigid_dynamic->getScene() != nullptr) {
        _rigid_dynamic->getScene()->removeActor(*_rigid_dynamic);
      }

      _rigid_dynamic->release();
    }

    _rigid_dynamic = nullptr;
  }
}

/**
 * Called when something other than the PhysX simulation caused the transform
 * of the node to change.  Synchronizes the node's new transform with the
 * associated PhysX actor.
 */
void PhysRigidDynamicNode::
do_transform_changed() {
  if (!is_kinematic()) {
    // Not a kinematic actor, just use the default behavior.
    PhysRigidBodyNode::do_transform_changed();
    return;
  }

  if (!get_sync_enabled()) {
    return;
  }

  // NOTE: It is invalid to call setKinematicTarget if the actor is
  // not in a scene.
  if (_rigid_dynamic->getScene() != nullptr) {
    NodePath np = NodePath::any_path((PandaNode *)this);
    CPT(TransformState) net_transform = np.get_net_transform();
    _rigid_dynamic->setKinematicTarget(panda_trans_to_physx(net_transform));
  }
}

/**
 * Initializes a dynamic node from an existing PxActor.
 */
PhysRigidDynamicNode::
PhysRigidDynamicNode(physx::PxRigidDynamic *actor) :
  PhysRigidBodyNode("dynamic") {
  _rigid_dynamic = actor;
  _rigid_dynamic->userData = this;
  physx::PxShape **shapes = (physx::PxShape **)alloca(sizeof(physx::PxShape *) * actor->getNbShapes());
  actor->getShapes(shapes, actor->getNbShapes());
  for (physx::PxU32 i = 0; i < actor->getNbShapes(); i++) {
    physx::PxShape *pxshape = shapes[i];
    if (pxshape->userData != nullptr) {
      _shapes.push_back((PhysShape *)pxshape->userData);
    } else {
      PT(PhysShape) shape = new PhysShape(pxshape);
      _shapes.push_back(shape);
    }
  }
  update_shape_filter_data();
  //_sync_enabled = false;
  _self_created = false;
  mark_internal_bounds_stale();
}

/**
 *
 */
physx::PxRigidBody *PhysRigidDynamicNode::
get_rigid_body() const {
  return _rigid_dynamic;
}

/**
 *
 */
void PhysRigidDynamicNode::
on_new_scene() {
  // The actor just got added to the scene.
  // If we're kinematic, synchronize the kinematic target to the node
  // position immediately.  We can't update the kinematic target while
  // the actor is not in a scene.
  if (is_kinematic()) {
    sync_transform();
  }
}
