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
 * Changes the collision mask of the node.  Each bit represents a collision
 * group.
 */
void PhysRigidActorNode::
set_into_collide_mask(CollideMask mask) {
  PandaNode::set_into_collide_mask(mask);

  // Update all shapes to contain the new collision mask.
  physx::PxU32 num_shapes = get_rigid_actor()->getNbShapes();
  physx::PxShape **shapes = (physx::PxShape **)alloca(sizeof(physx::PxShape *) * num_shapes);
  get_rigid_actor()->getShapes(shapes, num_shapes);
  for (physx::PxU32 i = 0; i < num_shapes; i++) {
    physx::PxShape *shape = shapes[i];
    physx::PxFilterData data = shape->getSimulationFilterData();
    physx::PxFilterData qdata = shape->getQueryFilterData();
    data.word0 = mask.get_word();
    qdata.word0 = mask.get_word();
    shape->setSimulationFilterData(data);
    shape->setQueryFilterData(data);
  }
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
set_collide_with(PhysRigidActorNode *other, bool flag) {
  do_set_collide_with(other, flag);
  other->do_set_collide_with(this, flag);
}

/**
 *
 */
void PhysRigidActorNode::
do_set_collide_with(PhysRigidActorNode *other, bool flag) {
  if (!flag) {
    Actors::const_iterator it = std::find(_no_collisions.begin(), _no_collisions.end(), other);
    if (it == _no_collisions.end()) {
      _no_collisions.push_back(other);
    }

  } else {
    Actors::const_iterator it = std::find(_no_collisions.begin(), _no_collisions.end(), other);
    if (it != _no_collisions.end()) {
      _no_collisions.erase(it);
    }
  }
}

/**
 *
 */
CollideMask PhysRigidActorNode::
get_legal_collide_mask() const {
  return CollideMask::all_on();
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
