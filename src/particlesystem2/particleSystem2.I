/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem2.I
 * @author brian
 * @date 2022-04-04
 */

/**
 * Returns the particle pool size of the system.  This is the maximum number
 * of particles that can be alive simultaneously.  Memory for each particle in
 * the pool is pre-allocated.
 */
INLINE int ParticleSystem2::
get_pool_size() const {
  return _pool_size;
}

/**
 * Returns the number of emitters in the system.
 */
INLINE int ParticleSystem2::
get_num_emitters() const {
  return (int)_emitters.size();
}

/**
 * Returns the nth emitter in the system.
 */
INLINE ParticleEmitter2 *ParticleSystem2::
get_emitter(int n) const {
  nassertr(n >= 0 && n < (int)_emitters.size(), nullptr);
  return _emitters[n];
}

/**
 * Returns the number of renderers in the system.
 */
INLINE int ParticleSystem2::
get_num_renderers() const {
  return (int)_renderers.size();
}

/**
 * Returns the nth renderer in the system.
 */
INLINE ParticleRenderer2 *ParticleSystem2::
get_renderer(int n) const {
  nassertr(n >= 0 && n < (int)_renderers.size(), nullptr);
  return _renderers[n];
}

/**
 * Returns the number of initializers in the system.
 */
INLINE int ParticleSystem2::
get_num_initializers() const {
  return (int)_initializers.size();
}

/**
 * Returns the nth initializer in the system.
 */
INLINE ParticleInitializer2 *ParticleSystem2::
get_initializer(int n) const {
  nassertr(n >= 0 && n < (int)_initializers.size(), nullptr);
  return _initializers[n];
}

/**
 * Returns the number of functions in the system.
 */
INLINE int ParticleSystem2::
get_num_functions() const {
  return (int)_functions.size();
}

/**
 * Returns the nth function in the system.
 */
INLINE ParticleFunction2 *ParticleSystem2::
get_function(int n) const {
  nassertr(n >= 0 && n < (int)_functions.size(), nullptr);
  return _functions[n];
}

/**
 * Returns the number of forces in the system.
 */
INLINE int ParticleSystem2::
get_num_forces() const {
  return (int)_forces.size();
}

/**
 * Returns the nth force in the system.
 */
INLINE ParticleForce2 *ParticleSystem2::
get_force(int n) const {
  nassertr(n >= 0 && n < (int)_forces.size(), nullptr);
  return _forces[n];
}

/**
 * Returns the number of constraints in the system.
 */
INLINE int ParticleSystem2::
get_num_constraints() const {
  return (int)_constraints.size();
}

/**
 * Returns the nth constraint in the system.
 */
INLINE ParticleConstraint2 *ParticleSystem2::
get_constraint(int n) const {
  nassertr(n >= 0 && n < (int)_constraints.size(), nullptr);
  return _constraints[n];
}

/**
 * Returns the number of particle systems that are children of this system.
 */
INLINE int ParticleSystem2::
get_num_children() const {
  return (int)_children.size();
}

/**
 * Returns the nth child particle system of the system.
 */
INLINE ParticleSystem2 *ParticleSystem2::
get_child(int n) const {
  nassertr(n >= 0 && n < (int)_children.size(), nullptr);
  return _children[n];
}

/**
 * Returns the number of input nodes to the system.
 */
INLINE int ParticleSystem2::
get_num_inputs() const {
  return (int)_inputs.size();
}

/**
 * Returns the input NodePath at the indicated index.
 */
INLINE const NodePath &ParticleSystem2::
get_input(int n) const {
  static NodePath fail_path = NodePath::fail();
  nassertr(n >= 0 && n < (int)_inputs.size(), fail_path);
  return _inputs[n];
}

/**
 * Returns the value of the input at the given index.
 *
 * This is the transform of the input node in particle system space.
 *
 * If there is no input at the given index, returns the identity
 * TransformState.
 */
INLINE const TransformState *ParticleSystem2::
get_input_value(int n) const {
  static CPT(TransformState) ident_ts = TransformState::make_identity();
  if (n < 0 || n >= (int)_input_values.size()) {
    return ident_ts;
  }
  return _input_values[n];
}

/**
 * Returns the parent scene graph node of the particle system.
 * The particle system renders and operates relative to this node.
 */
INLINE const NodePath &ParticleSystem2::
get_parent_node() const {
  return _parent;
}

/**
 * Returns the NodePath that the particle system node should follow.
 * This allows a particle system to be parented to one node, but have it
 * follow only the position of another node, without inheriting render
 * state or other transform from the follow node.
 */
INLINE const NodePath &ParticleSystem2::
get_follow_parent() const {
  return _follow_parent;
}

/**
 * Returns the scene graph node representing this particle system.
 * It is parented to the NodePath returned by get_parent_node().
 */
INLINE const NodePath &ParticleSystem2::
get_node() const {
  return _np;
}

/**
 * Returns true if the particle system is currently running/active.
 */
INLINE bool ParticleSystem2::
is_running() const {
  return _running;
}

/**
 * Returns the amount of time in seconds that the particle system has been
 * running/active for.  This has no meaning if the system is not currently
 * running.
 */
INLINE double ParticleSystem2::
get_elapsed_time() const {
  return _elapsed;
}

/**
 * Returns the number of particles in the system that are currently alive
 * and being simulated.
 *
 * This will never be higher than the value returned by `get_pool_size()`.
 *
 * Returns 0 if the system is not currently running.
 */
INLINE int ParticleSystem2::
get_num_alive_particles() const {
  return _num_alive_particles;
}
