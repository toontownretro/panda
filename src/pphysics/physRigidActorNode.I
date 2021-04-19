/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidActorNode.I
 * @author brian
 * @date 2021-04-14
 */

/**
 * Adds the indicated shape to the actor.
 */
INLINE void PhysRigidActorNode::
add_shape(PhysShape *shape) {
  nassertv(shape->get_shape() != nullptr);
  physx::PxFilterData data;
  data.word0 = get_into_collide_mask().get_word();
  shape->get_shape()->setSimulationFilterData(data);
  shape->get_shape()->setQueryFilterData(data);
  get_rigid_actor()->attachShape(*shape->get_shape());
}

/**
 * Removes the indicated shape from the actor.
 */
INLINE void PhysRigidActorNode::
remove_shape(PhysShape *shape) {
  nassertv(shape->get_shape() != nullptr);
  get_rigid_actor()->detachShape(*shape->get_shape());
}

/**
 * Returns the number of shapes present on the actor.
 */
INLINE size_t PhysRigidActorNode::
get_num_shapes() const {
  return get_rigid_actor()->getNbShapes();
}

/**
 * Returns the nth shape on the actor.
 */
INLINE PhysShape *PhysRigidActorNode::
get_shape(size_t n) const {
  physx::PxRigidActor *actor = get_rigid_actor();
  nassertr(n < actor->getNbShapes(), nullptr);

  physx::PxShape *shape;
  actor->getShapes(&shape, 1, n);

  return (PhysShape *)shape->userData;
}

/**
 * Sets whether or not a change to the node's transform should be automatically
 * synchronized with the associated PhysX actor.
 */
INLINE void PhysRigidActorNode::
set_sync_enabled(bool flag) {
  _sync_enabled = flag;
}

/**
 *
 */
INLINE bool PhysRigidActorNode::
get_sync_enabled() const {
  return _sync_enabled;
}
