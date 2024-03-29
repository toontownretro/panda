/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidDynamicNode.I
 * @author brian
 * @date 2021-04-14
 */

/**
 *
 */
INLINE void PhysRigidDynamicNode::
wake_up() {
  _rigid_dynamic->wakeUp();
}

/**
 *
 */
INLINE void PhysRigidDynamicNode::
put_to_sleep() {
  _rigid_dynamic->putToSleep();
}

/**
 *
 */
INLINE bool PhysRigidDynamicNode::
is_sleeping() const {
  return _rigid_dynamic->isSleeping();
}

/**
 *
 */
INLINE void PhysRigidDynamicNode::
set_sleep_threshold(PN_stdfloat threshold) {
  _rigid_dynamic->setSleepThreshold((physx::PxReal)threshold);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidDynamicNode::
get_sleep_threshold() const {
  return (PN_stdfloat)_rigid_dynamic->getSleepThreshold();
}

/**
 *
 */
INLINE void PhysRigidDynamicNode::
set_stabilization_threshold(PN_stdfloat threshold) {
  _rigid_dynamic->setStabilizationThreshold((physx::PxReal)threshold);
}

/**
 *
 */
INLINE PN_stdfloat PhysRigidDynamicNode::
get_stabilization_threshold() const {
  return (PN_stdfloat)_rigid_dynamic->getStabilizationThreshold();
}

/**
 *
 */
INLINE void PhysRigidDynamicNode::
set_num_position_iterations(unsigned int count) {
  _rigid_dynamic->setSolverIterationCounts(count, get_num_velocity_iterations());
}

/**
 *
 */
INLINE unsigned int PhysRigidDynamicNode::
get_num_position_iterations() const {
  physx::PxU32 pos, vel;
  _rigid_dynamic->getSolverIterationCounts(pos, vel);
  return pos;
}

/**
 *
 */
INLINE void PhysRigidDynamicNode::
set_num_velocity_iterations(unsigned int count) {
  _rigid_dynamic->setSolverIterationCounts(get_num_position_iterations(), count);
}

/**
 *
 */
INLINE unsigned int PhysRigidDynamicNode::
get_num_velocity_iterations() const {
  physx::PxU32 pos, vel;
  _rigid_dynamic->getSolverIterationCounts(pos, vel);
  return vel;
}
