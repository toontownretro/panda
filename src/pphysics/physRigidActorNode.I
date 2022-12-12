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
  get_rigid_actor()->attachShape(*shape->get_shape());
  if (std::find(_shapes.begin(), _shapes.end(), shape) == _shapes.end()) {
    _shapes.push_back(shape);
    update_shape_filter_data(_shapes.size() - 1);
  }
  mark_internal_bounds_stale();
}

/**
 * Removes the indicated shape from the actor.
 */
INLINE void PhysRigidActorNode::
remove_shape(PhysShape *shape) {
  nassertv(shape->get_shape() != nullptr);
  get_rigid_actor()->detachShape(*shape->get_shape());
  auto it = std::find(_shapes.begin(), _shapes.end(), shape);
  if (it != _shapes.end()) {
    _shapes.erase(it);
  }
  mark_internal_bounds_stale();
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
  nassertr(n >= 0 && n < _shapes.size(), nullptr);
  return _shapes[n];
}

/**
 * Sets the callback to be executed when this node wakes up.
 */
INLINE void PhysRigidActorNode::
set_wake_callback(CallbackObject *obj) {
  _wake_callback = obj;
  get_rigid_actor()->setActorFlag(
    physx::PxActorFlag::eSEND_SLEEP_NOTIFIES,
    (_wake_callback != nullptr) || (_sleep_callback != nullptr));
}

/**
 * Returns the callback to be executed when this node wakes up.
 */
INLINE CallbackObject *PhysRigidActorNode::
get_wake_callback() const {
  return _wake_callback;
}

/**
 * Sets the callback to be executed when this node falls asleep.
 */
INLINE void PhysRigidActorNode::
set_sleep_callback(CallbackObject *obj) {
  _sleep_callback = obj;
  get_rigid_actor()->setActorFlag(
    physx::PxActorFlag::eSEND_SLEEP_NOTIFIES,
    (_wake_callback != nullptr) || (_sleep_callback != nullptr));
}

/**
 * Returns the callback to be executed when this node falls asleep.
 */
INLINE CallbackObject *PhysRigidActorNode::
get_sleep_callback() const {
  return _wake_callback;
}

/**
 * Sets the callback to be executed when another actor enters or exits this
 * node's trigger volume.
 */
INLINE void PhysRigidActorNode::
set_trigger_callback(CallbackObject *obj) {
  _trigger_callback = obj;
}

/**
 * Returns the callback to be executed when another actor enters or exits this
 * node's trigger volume.
 */
INLINE CallbackObject *PhysRigidActorNode::
get_trigger_callback() const {
  return _trigger_callback;
}

/**
 * Sets the callback to be executed when another actor comes in contact with
 * this node.
 */
INLINE void PhysRigidActorNode::
set_contact_callback(CallbackObject *obj) {
  _contact_callback = obj;
}

/**
 * Returns the callback to be executed when another actor comes in contact with
 * this node.
 */
INLINE CallbackObject *PhysRigidActorNode::
get_contact_callback() const {
  return _contact_callback;
}

/**
 * Sets the callback to be executed when this node moves through space during
 * the simulation.
 */
INLINE void PhysRigidActorNode::
set_advance_callback(CallbackObject *obj) {
  _advance_callback = obj;
}

/**
 * Returns the callback to be executed when this node moves through space
 * during the simulation.
 */
INLINE CallbackObject *PhysRigidActorNode::
get_advance_callback() const {
  return _advance_callback;
}

/**
 * Sets the callback that should be invoked to perform custom filtering
 * logic for contact generation between this actor and other actors in
 * the scene.
 */
INLINE void PhysRigidActorNode::
set_contact_filter(CallbackObject *filter) {
  _contact_filter = filter;
}

/**
 * Clears the contact filter.  See set_contact_filter().
 */
INLINE void PhysRigidActorNode::
clear_contact_filter() {
  _contact_filter = nullptr;
}

/**
 * Returns the callback that should perform custom filtering logic for
 * contact generation between this actor and other actors in the scene.
 */
INLINE CallbackObject *PhysRigidActorNode::
get_contact_filter() const {
  return _contact_filter;
}

/**
 *
 */
INLINE bool PhysRigidActorNode::
has_no_collide_with(PhysRigidActorNode *other) const {
  return std::find(_no_collisions.begin(), _no_collisions.end(), other) != _no_collisions.end();
}

/**
 * Returns the contents mask of the actor node.
 */
INLINE BitMask32 PhysRigidActorNode::
get_from_collide_mask() const {
  return _from_collide_mask;
}

/**
 * Returns the solid mask of the actor node.  This is a bitmask of contents
 * that are solid to the actor.
 */
INLINE BitMask32 PhysRigidActorNode::
get_into_collide_mask() const {
  return _into_collide_mask;
}

/**
 *
 */
INLINE void PhysRigidActorNode::
set_simulation_disabled(bool flag) {
  get_rigid_actor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, flag);
}

/**
 *
 */
INLINE bool PhysRigidActorNode::
get_simulation_disabled() const {
  return get_rigid_actor()->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION);
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
