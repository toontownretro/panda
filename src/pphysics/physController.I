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
  _collision_flags(CF_none),
  _group_mask(CollideMask::all_on())
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
  get_controller()->setPosition(Vec3_to_PxExVec3(pos));
}

/**
 *
 */
INLINE LPoint3 PhysController::
get_position() const {
  return PxExVec3_to_Vec3(get_controller()->getPosition());
}

/**
 *
 */
INLINE void PhysController::
set_foot_position(const LPoint3 &position) {
  get_controller()->setFootPosition(Vec3_to_PxExVec3(position));
}

/**
 *
 */
INLINE LPoint3 PhysController::
get_foot_position() const {
  return PxExVec3_to_Vec3(get_controller()->getFootPosition());
}

/**
 *
 */
INLINE void PhysController::
set_step_offset(PN_stdfloat offset) {
  get_controller()->setStepOffset(offset);
}

/**
 *
 */
INLINE PN_stdfloat PhysController::
get_step_offset() const {
  return get_controller()->getStepOffset();
}

/**
 *
 */
INLINE void PhysController::
set_contact_offset(PN_stdfloat offset) {
  get_controller()->setContactOffset(offset);
}

/**
 *
 */
INLINE PN_stdfloat PhysController::
get_contact_offset() const {
  return get_controller()->getContactOffset();
}

/**
 *
 */
INLINE void PhysController::
set_up_direction(const LVector3 &dir) {
  get_controller()->setUpDirection(Vec3_to_PxVec3(dir));
}

/**
 *
 */
INLINE LVector3 PhysController::
get_up_direction() const {
  return PxVec3_to_Vec3(get_controller()->getUpDirection());
}

/**
 * Sets the slope limit for the character, expressed in degrees.
 */
INLINE void PhysController::
set_slope_limit(PN_stdfloat limit) {
  get_controller()->setSlopeLimit(std::cos(deg_2_rad(limit)));
}

/**
 * Returns the slope limit for the character, expressed in degrees.
 */
INLINE PN_stdfloat PhysController::
get_slope_limit() const {
  return rad_2_deg(std::acos(get_controller()->getSlopeLimit()));
}

/**
 *
 */
INLINE void PhysController::
resize(PN_stdfloat size) {
  get_controller()->resize(size);
}

/**
 * Sets the collision group mask of the controller.  The controller will
 * collide with other groups that have collisions enabled between the group and
 * the controller's group.
 */
INLINE void PhysController::
set_collide_mask(CollideMask mask) {
  _group_mask = mask;
}

/**
 * Returns the collision group mask of the controller.
 */
INLINE CollideMask PhysController::
get_collide_mask() const {
  return _group_mask;
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
