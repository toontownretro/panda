/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidStaticNode.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSRIGIDSTATICNODE_H
#define PHYSRIGIDSTATICNODE_H

#include "pandabase.h"
#include "physRigidActorNode.h"

/**
 * A rigid body that is intended to be completely stationary in the scene.  Use
 * this for non-moving level geometry and such.
 */
class EXPCL_PANDA_PPHYSICS PhysRigidStaticNode final : public PhysRigidActorNode {
PUBLISHED:
  PhysRigidStaticNode(const std::string &name);
  ~PhysRigidStaticNode();

public:
  virtual physx::PxRigidActor *get_rigid_actor() const override;

private:
  physx::PxRigidStatic *_rigid_static;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysRigidActorNode::init_type();
    register_type(_type_handle, "PhysRigidStaticNode",
                  PhysRigidActorNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physRigidStaticNode.I"

#endif // PHYSRIGIDSTATICNODE_H
