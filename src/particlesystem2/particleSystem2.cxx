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
 *
 */
void ParticleSystem2::
set_pool_size(int size) {
  nassertv(!_running);
  _pool_size = size;
}

/**
 *
 */
void ParticleSystem2::
add_emitter(ParticleEmitter2 *emitter) {
  _emitters.push_back(emitter);
}

/**
 *
 */
void ParticleSystem2::
add_renderer(ParticleRenderer2 *renderer) {
  _renderers.push_back(renderer);
}

/**
 *
 */
void ParticleSystem2::
add_initializer(ParticleInitializer2 *initializer) {
  _initializers.push_back(initializer);
}

/**
 *
 */
void ParticleSystem2::
add_function(ParticleFunction2 *func) {
  _functions.push_back(func);
}

/**
 *
 */
void ParticleSystem2::
add_force(ParticleForce2 *force) {
  _forces.push_back(force);
}

/**
 *
 */
void ParticleSystem2::
add_child(ParticleSystem2 *child) {
  _children.push_back(child);
}

/**
 *
 */
void ParticleSystem2::
start(const NodePath &parent, double time) {
  nassertv(!_running);
  nassertv(_pool_size > 0);

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
