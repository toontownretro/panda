/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physScene.I
 * @author brian
 * @date 2021-04-14
 */

/**
 * Sets the gravity vector of the scene.
 */
INLINE void PhysScene::
set_gravity(const LVector3 &gravity) {
  nassertv(_scene != nullptr);

  _scene->setGravity(panda_vec_to_physx(gravity));
}

/**
 * Returns the gravity vector of the scene.
 */
INLINE LVector3 PhysScene::
get_gravity() const {
  nassertr(_scene != nullptr, LVector3());

  physx::PxVec3 gravity = _scene->getGravity();
  return physx_vec_to_panda(gravity);
}

/**
 * Shifts the origin point of the scene.  May be useful in large scenes to
 * avoid floating-point imprecision when the viewpoint is far away from the
 * origin.
 */
INLINE void PhysScene::
shift_origin(const LVector3 &shift) {
  nassertv(_scene != nullptr);

  _scene->shiftOrigin(panda_vec_to_physx(shift));
}

/**
 * Sets the fixed timestep of the simulation.
 */
INLINE void PhysScene::
set_fixed_timestep(double step) {
  _fixed_timestep = step;
}

/**
 * Returns the fixed timestep of the simulation.
 *
 * See set_fixed_timestep().
 */
INLINE double PhysScene::
get_fixed_timestep() const {
  return _fixed_timestep;
}

/**
 * Sets the maximum number of substeps that are allowed to be simulated.
 *
 * This can prevent the simulation from grinding to a halt if we can't keep up.
 */
INLINE void PhysScene::
set_max_substeps(int count) {
  _max_substeps = count;
}

/**
 * Returns the maximum number of simulation substeps.
 *
 * See set_max_substeps().
 */
INLINE int PhysScene::
get_max_substeps() const {
  return _max_substeps;
}

/**
 * Enables or disables collisions between the two indicated collision groups.
 */
INLINE void PhysScene::
set_group_collision_flag(int a, int b, bool enable) {
  PandaSimulationFilterShader::set_group_collision_flag(a, b, enable);
}

/**
 * Returns whether or not collisions are enabled between the two indicated
 * collision groups.
 */
INLINE bool PhysScene::
get_group_collision_flag(int a, int b) const {
  return PandaSimulationFilterShader::get_group_collision_flag(a, b);
}

/**
 * Returns true if there is a global contact event from the last simulation
 * run that is awaiting processing.
 */
INLINE bool PhysScene::
has_contact_event() const {
  return !_global_contact_queue.empty();
}

/**
 * Pops and returns the next global contact event from the last simulation run.
 */
INLINE PT(RefCallbackData) PhysScene::
pop_contact_event() {
  nassertr(!_global_contact_queue.empty(), nullptr);
  PT(RefCallbackData) data = _global_contact_queue.front();
  _global_contact_queue.pop_front();
  return data;
}

/**
 * Enqueues a scene-wide contact event for processing in show code after
 * the simulation finishes.
 */
INLINE void PhysScene::
enqueue_global_contact(RefCallbackData *data) {
  _global_contact_queue.push_back(data);
}

/**
 *
 */
void PhysScene::
enqueue_callback(CallbackObject *obj, RefCallbackData *data) {
  Callback clbk;
  clbk._callback = obj;
  clbk._data = data;
  _callbacks.push_back(clbk);
}

/**
 * Returns the underlying PxScene object.
 */
INLINE physx::PxScene *PhysScene::
get_scene() const {
  return _scene;
}

/**
 * Returns the PhysX controller manager for the scene.
 */
INLINE physx::PxControllerManager *PhysScene::
get_controller_manager() const {
  return _controller_mgr;
}
