/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physController.I
 * @author brian
 * @date 2021-04-27
 */

/**
 * Returns the PhysController that the hit pertains to.
 */
INLINE PhysController *PhysControllerHitData::
get_controller() const {
  return _controller;
}

/**
 * Returns the world position of the hit.
 */
INLINE const LPoint3 &PhysControllerHitData::
get_world_pos() const {
  return _world_pos;
}

/**
 * Returns the world normal of the hit.
 */
INLINE const LVector3 &PhysControllerHitData::
get_world_normal() const {
  return _world_normal;
}

/**
 *
 */
INLINE const LVector3 &PhysControllerHitData::
get_motion_dir() const {
  return _motion_dir;
}

/**
 *
 */
INLINE PN_stdfloat PhysControllerHitData::
get_motion_length() const {
  return _motion_length;
}

/**
 *
 */
INLINE PhysShape *PhysControllerShapeHitData::
get_shape() const {
  return _shape;
}

/**
 *
 */
INLINE PhysRigidActorNode *PhysControllerShapeHitData::
get_actor() const {
  return _actor;
}

/**
 *
 */
INLINE uint32_t PhysControllerShapeHitData::
get_triangle_index() const {
  return _triangle_index;
}

/**
 *
 */
INLINE PhysController *PhysControllersHitData::
get_other_controller() const {
  return _other;
}

/**
 *
 */
INLINE PhysControllerHitCallback *PhysControllerHitCallback::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new PhysControllerHitCallback;
  }

  return _global_ptr;
}

/**
 *
 */
INLINE PhysController::
PhysController() :
  _collision_flags(CF_none)
{
}

/**
 *
 */
INLINE PhysController::ShapeType PhysController::
get_shape_type() const {
  return (ShapeType)get_controller()->getType();
}

/**
 *
 */
INLINE void PhysController::
set_position(const LPoint3 &pos) {
  get_controller()->setPosition(panda_vec_to_physx_ex(pos));
}

/**
 *
 */
INLINE LPoint3 PhysController::
get_position() const {
  return physx_ex_vec_to_panda(get_controller()->getPosition());
}

/**
 *
 */
INLINE void PhysController::
set_foot_position(const LPoint3 &position) {
  get_controller()->setFootPosition(panda_vec_to_physx_ex(position));
}

/**
 *
 */
INLINE LPoint3 PhysController::
get_foot_position() const {
  return physx_ex_vec_to_panda(get_controller()->getFootPosition());
}

/**
 *
 */
INLINE void PhysController::
set_step_offset(PN_stdfloat offset) {
  get_controller()->setStepOffset(panda_length_to_physx(offset));
}

/**
 *
 */
INLINE PN_stdfloat PhysController::
get_step_offset() const {
  return physx_length_to_panda(get_controller()->getStepOffset());
}

/**
 *
 */
INLINE void PhysController::
set_contact_offset(PN_stdfloat offset) {
  get_controller()->setContactOffset(panda_length_to_physx(offset));
}

/**
 *
 */
INLINE PN_stdfloat PhysController::
get_contact_offset() const {
  return physx_length_to_panda(get_controller()->getContactOffset());
}

/**
 *
 */
INLINE void PhysController::
set_up_direction(const LVector3 &dir) {
  get_controller()->setUpDirection(panda_norm_vec_to_physx(dir));
}

/**
 *
 */
INLINE LVector3 PhysController::
get_up_direction() const {
  return physx_norm_vec_to_panda(get_controller()->getUpDirection());
}

/**
 * Sets the slope limit for the character, expressed in degrees.
 */
INLINE void PhysController::
set_slope_limit(PN_stdfloat limit) {
  get_controller()->setSlopeLimit(std::cos(panda_ang_to_physx(limit)));
}

/**
 * Returns the slope limit for the character, expressed in degrees.
 */
INLINE PN_stdfloat PhysController::
get_slope_limit() const {
  return physx_ang_to_panda(std::acos(get_controller()->getSlopeLimit()));
}

/**
 * Sets the solid mask of the controller.  This is a bitmask of contents that
 * are solid to the controller.
 */
INLINE void PhysController::
set_into_collide_mask(BitMask32 mask) {
  nassertv(_actor_node != nullptr);
  _actor_node->set_into_collide_mask(mask);
}

/**
 * Returns the solid mask of the controller.  This is a bitmask of contents
 * that are solid to the controller.
 */
INLINE BitMask32 PhysController::
get_into_collide_mask() const {
  nassertr(_actor_node != nullptr, 0)
  return _actor_node->get_into_collide_mask();
}

/**
 * Returns the mask of contents of the controller.  See set_from_collide_mask().
 */
INLINE BitMask32 PhysController::
get_from_collide_mask() const {
  nassertr(_actor_node != nullptr, 0)
  return _actor_node->get_from_collide_mask();
}

/**
 * Returns the underlying actor node of the controller.
 */
INLINE PhysRigidDynamicNode *PhysController::
get_actor_node() const {
  return _actor_node;
}

/**
 * Returns the shape of the underlying actor of the controller.
 */
INLINE PhysShape *PhysController::
get_actor_shape() const {
  nassertr(_actor_node != nullptr, nullptr);
  return _actor_node->get_shape(0);
}

/**
 * Returns the collision flags from the most recent move() call.
 */
INLINE unsigned int PhysController::
get_collision_flags() const {
  return _collision_flags;
}

/**
 * Returns the number of shapes the character had collisions with in the last
 * move() call.
 */
INLINE size_t PhysController::
get_num_shape_hits() const {
  return _shape_hits.size();
}

/**
 * Returns the shape hit data at the indicated index.
 */
INLINE const PhysControllerShapeHitData *PhysController::
get_shape_hit(size_t n) const {
  nassertr(n >= 0 && n < _shape_hits.size(), nullptr);
  return &_shape_hits[n];
}

/**
 * Returns the number of other characters the character had collisions with in
 * the last move() call.
 */
INLINE size_t PhysController::
get_num_controller_hits() const {
  return _controller_hits.size();
}

/**
 * Returns the controller hit data at the indicated index.
 */
INLINE const PhysControllersHitData *PhysController::
get_controller_hit(size_t n) const {
  nassertr(n >= 0 && n < _controller_hits.size(), nullptr);
  return &_controller_hits[n];
}
