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

/**
 *
 */
PhysRigidActorNode::
PhysRigidActorNode(const std::string &name) :
  PandaNode(name)
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
