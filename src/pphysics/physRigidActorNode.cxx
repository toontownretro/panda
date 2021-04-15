/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidActorNode.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "physRigidActorNode.h"
#include "physScene.h"

#include "nodePath.h"

TypeHandle PhysRigidActorNode::_type_handle;

/**
 *
 */
PhysRigidActorNode::
PhysRigidActorNode(const std::string &name) :
  PandaNode(name),
  _sync_enabled(true)
{
}

/**
 * Adds this node into the indicated PhysScene.
 */
void PhysRigidActorNode::
add_to_scene(PhysScene *scene) {
  scene->get_scene()->addActor(*get_rigid_actor());
}

/**
 * Removes this node from the indicated PhysScene.
 */
void PhysRigidActorNode::
remove_from_scene(PhysScene *scene) {
  scene->get_scene()->removeActor(*get_rigid_actor());
}

/**
 *
 */
void PhysRigidActorNode::
parents_changed() {
  if (get_num_parents() > 0) {
    do_transform_changed();
  }
}

/**
 *
 */
void PhysRigidActorNode::
transform_changed() {
  do_transform_changed();
}

/**
 * Called when something other than the PhysX simulation caused the transform
 * of the node to change.  Synchronizes the node's new transform with the
 * associated PhysX actor.
 */
void PhysRigidActorNode::
do_transform_changed() {
  if (!_sync_enabled) {
    return;
  }

  NodePath np = NodePath::any_path((PandaNode *)this);

  CPT(TransformState) net_transform = np.get_net_transform();
  const LPoint3 &pos = net_transform->get_pos();
  const LQuaternion &quat = net_transform->get_quat();

  physx::PxTransform pose(
    physx::PxVec3(pos[0], pos[1], pos[2]),
    physx::PxQuat(quat[1], quat[2], quat[3], quat[0]));

  get_rigid_actor()->setGlobalPose(pose);
}
