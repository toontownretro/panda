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
 *
 */
INLINE void PhysController::
set_slope_limit(PN_stdfloat limit) {
  get_controller()->setSlopeLimit(limit);
}

/**
 *
 */
INLINE PN_stdfloat PhysController::
get_slope_limit() const {
  return get_controller()->getSlopeLimit();
}

/**
 *
 */
void PhysController::
resize(PN_stdfloat size) {
  get_controller()->resize(size);
}
