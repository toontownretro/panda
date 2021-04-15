/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidActorNode.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSRIGIDACTORNODE_H
#define PHYSRIGIDACTORNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "physShape.h"

class PhysScene;

#include "physx_includes.h"

/**
 * Base class for rigid (non-deformable) objects in a scene.
 */
class EXPCL_PANDA_PPHYSICS PhysRigidActorNode : public PandaNode {
PUBLISHED:
  INLINE void add_shape(PhysShape *shape);
  INLINE void remove_shape(PhysShape *shape);

  INLINE size_t get_num_shapes() const;
  INLINE PhysShape *get_shape(size_t n) const;

  void add_to_scene(PhysScene *scene);
  void remove_from_scene(PhysScene *scene);

  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);
  MAKE_SEQ_PROPERTY(shapes, get_num_shapes, get_shape);

protected:
  PhysRigidActorNode(const std::string &name);

public:
  virtual physx::PxRigidActor *get_rigid_actor() const = 0;
};


#include "physRigidActorNode.I"

#endif // PHYSRIGIDACTORNODE_H
