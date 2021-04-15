/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidDynamicNode.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSRIGIDDYNAMICNODE_H
#define PHYSRIGIDDYNAMICNODE_H

#include "pandabase.h"
#include "physRigidBodyNode.h"

/**
 * A rigid body that moves around and reacts with other actors in the scene.
 */
class EXPCL_PANDA_PPHYSICS PhysRigidDynamicNode final : public PhysRigidBodyNode {
PUBLISHED:
  PhysRigidDynamicNode(const std::string &name);
  ~PhysRigidDynamicNode();

  INLINE void wake_up();

  INLINE void put_to_sleep();
  INLINE bool is_sleeping() const;

  INLINE void set_sleep_threshold(PN_stdfloat threshold);
  INLINE PN_stdfloat get_sleep_threshold() const;

  INLINE void set_stabilization_threshold(PN_stdfloat threshold);
  INLINE PN_stdfloat get_stabilization_threshold() const;

public:
  virtual physx::PxRigidBody *get_rigid_body() const override;

private:
  physx::PxRigidDynamic *_rigid_dynamic;
};

#include "physRigidDynamicNode.I"



#endif // PHYSRIGIDDYNAMICNODE_H
