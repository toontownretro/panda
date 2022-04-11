/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem2.cxx
 * @author brian
 * @date 2022-04-04
 */

#include "particleSystem2.h"

TypeHandle ParticleSystem2::_type_handle;

/**
 *
 */
ParticleSystem2::
ParticleSystem2() :
  _elapsed(0.0),
  _running(false),
  _pool_size(256),
  _num_alive_particles(0),
  _dt(0.0),
  _prev_dt(0.05)
{
}

/**
 * Sets the maximum number of particles that can be simulated simultaneously in
 * the system.  Memory for `size` particles is pre-allocated when the system
 * starts.
 *
 * The pool should be big enough to hold the maximum number of particles that
 * may be simulated at the same time, at any point in the simulation of the
 * system.  For performance and memory considerations, it is important to keep
 * the pool size no bigger than actually needed.
 */
void ParticleSystem2::
set_pool_size(int size) {
  nassertv(!_running);
  _pool_size = size;
}

/**
 * Adds a new emitter to the particle system.  Emitters are responsible for
 * determining when to spawn new particles, how often, and how many.
 */
void ParticleSystem2::
add_emitter(ParticleEmitter2 *emitter) {
  _emitters.push_back(emitter);
}

/**
 * Adds a new renderer to the particle system.  Renderers create a visual
 * representation of the particle system.
 */
void ParticleSystem2::
add_renderer(ParticleRenderer2 *renderer) {
  _renderers.push_back(renderer);
}

/**
 * Adds a new initializer to the particle system.  Initializers are
 * responsible for setting up the initial values of particle attributes
 * when they spawn, such as position and velocity.
 */
void ParticleSystem2::
add_initializer(ParticleInitializer2 *initializer) {
  _initializers.push_back(initializer);
}

/**
 * Adds a new function to the particle system.  Functions define the behavior
 * of particles in the system, such as how they move and change appearance
 * over time.
 */
void ParticleSystem2::
add_function(ParticleFunction2 *func) {
  _functions.push_back(func);
}

/**
 * Adds a new physical force to the particle system.  The force will act
 * upon all particles in the system.  Note that the system needs a
 * LinearMotionParticleFunction for forces to have any effect.
 */
void ParticleSystem2::
add_force(ParticleForce2 *force) {
  _forces.push_back(force);
}

/**
 * Adds the given particle system as a child of this particle system.
 * The child will be started and stopped along with this system, and input
 * nodes set on this system will propagate down to the child.
 */
void ParticleSystem2::
add_child(ParticleSystem2 *child) {
  _children.push_back(child);
}

/**
 * Adds a new input node to the particle system.  Initializers and functions
 * may use the transform of this node to influence their behaviors.
 *
 * By convention, the first input node defines the emission coordinate space.
 */
void ParticleSystem2::
add_input(const NodePath &input) {
  _inputs.push_back(input);
  _input_values.push_back(TransformState::make_identity());
}

/**
 *
 */
void ParticleSystem2::
set_input(int n, const NodePath &input) {
  if (n >= _inputs.size()) {
    _inputs.resize(n + 1);
    _input_values.resize(n + 1);
  }
  _inputs[n] = input;
  _input_values[n] = TransformState::make_identity();
}

/**
 *
 */
void ParticleSystem2::
start(const NodePath &parent, double time) {
  nassertv(!_running);
  nassertv(_pool_size > 0);

  _parent = parent;

  _elapsed = time;

  _num_alive_particles = 0;

  // Resize to contain pool size particles.
  _particles.resize(_pool_size);
  for (int i = 0; i < (int)_particles.size(); ++i) {
    Particle *p = &_particles[i];
    p->_pos.fill(0.0f);
    p->_prev_pos.fill(0.0f);
    p->_velocity.fill(0.0f);
    p->_duration = 0.0f;
    p->_scale.fill(1.0f);
    p->_rotation_speed = 0.0f;
    p->_rotation = 0.0f;
    p->_color.fill(1.0f);
    p->_spawn_time = 0.0f;
    // Shouldn't be necessary, but just to be safe.
    p->_id = (size_t)i;
    p->_alive = false;

    // At the start, all particles are dead and free for use.
    _free_particles.push_back(i);
  }

  // Initialize our renderers.
  for (ParticleRenderer2 *renderer : _renderers) {
    renderer->initialize(parent, this);
  }

  _running = true;
}

/**
 *
 */
void ParticleSystem2::
stop() {
  nassertv(_running);

  // Shutdown our renderers.
  for (ParticleRenderer2 *renderer : _renderers) {
    renderer->shutdown(this);
  }

  _parent.clear();

  _running = false;
  _num_alive_particles = 0;
}

/**
 * Main particle system update routine.
 */
void ParticleSystem2::
update(double dt) {
  nassertv(_running);

  _dt = dt;

  nassertv(!_parent.is_empty());

  // Fetch current values of all dynamic input nodes.
  // This is the transform of the input node relative to the particle
  // system's render parent, or world-space as far as the particle system
  // is concerned.
  for (size_t i = 0; i < _inputs.size(); ++i) {
    if (!_inputs[i].is_empty()) {
      _input_values[i] = _inputs[i].get_transform(_parent);
    } else {
      _input_values[i] = TransformState::make_identity();
    }
  }

  // First update the emitter so they can birth new particles if necessary.
  for (ParticleEmitter2 *emitter : _emitters) {
    int birth_count = emitter->update(_elapsed);
    if (birth_count > 0) {
      // Emitter wants to birth some particles.
      birth_particles(birth_count);
    }
  }

  // Run all functions over each alive particle.
  for (ParticleFunction2 *func : _functions) {
    func->update(_elapsed, dt, this);
  }

  // TEMPORARY: update renderers.  This should be deferred to a
  // cull callback or something.
  for (ParticleRenderer2 *renderer : _renderers) {
    renderer->update(this);
  }

  // Update children.
  for (ParticleSystem2 *child : _children) {
    child->update(dt);
  }

  // Accumulate time.
  _elapsed += dt;

  _prev_dt = dt;
}

/**
 * Kills the particle at the indicated index.
 */
void ParticleSystem2::
kill_particle(int n) {
  // Sanity check index.
  nassertv(n >= 0 && n < (int)_particles.size());

  // Shouldn't already be killed.
  nassertv(std::find(_free_particles.begin(), _free_particles.end(), n) == _free_particles.end());

  Particle *p = &_particles[n];
  nassertv(p->_alive);
  p->_alive = false;

  --_num_alive_particles;

  // Throw index into free queue to reuse this particle for later
  // births.
  _free_particles.push_back(n);
}

/**
 * Births/spawns the given number of particles into the system.  Reuses
 * particles from the free queue.
 *
 * Returns true if the given number of particles were available to be spawned,
 * or false otherwise.
 *
 * Runs each initializer of the system on the new particles.
 */
bool ParticleSystem2::
birth_particles(int count) {
  count = std::min(count, (int)_free_particles.size());
  if (count == 0) {
    return false;
  }

  int *indices = (int *)alloca(sizeof(int) * count);

  for (int i = 0; i < count; ++i) {
    // Grab first free particle from pool.
    int particle_index = _free_particles.back();
    _free_particles.pop_back();

    // Sanity check index.
    nassertr(particle_index >= 0 && particle_index < (int)_particles.size(), false);

    // Reset some data.
    Particle *p = &_particles[particle_index];
    p->_pos.set(0, 0, 0);
    p->_prev_pos.set(0, 0, 0);
    p->_velocity.set(0, 0, 0);
    p->_duration = 0.0f;
    p->_scale.set(1.0f, 1.0f);
    p->_rotation_speed = 0.0f;
    p->_rotation = 0.0f;
    p->_color.set(1, 1, 1, 1);
    p->_spawn_time = _elapsed;
    // Shouldn't be necessary, but just to be safe.
    p->_id = (size_t)particle_index;
    p->_alive = true;

    ++_num_alive_particles;

    indices[i] = particle_index;
  }

  // Now run each initializer on the new particles.
  for (ParticleInitializer2 *init : _initializers) {
    init->init_particles(_elapsed, indices, count, this);
  }

  return true;
}
