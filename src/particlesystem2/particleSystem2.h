/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystem2.h
 * @author brian
 * @date 2022-04-04
 */

#ifndef PARTICLESYSTEM2_H
#define PARTICLESYSTEM2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pdeque.h"

#include "particle.h"
#include "particleEmitter2.h"
#include "particleFunction2.h"
#include "particleInitializer2.h"
#include "particleRenderer2.h"
#include "particleForce2.h"

/**
 * A particle system is a collection of particles, which are essentially
 * points in space with
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleSystem2 : public TypedWritableReferenceCount {
PUBLISHED:
  ParticleSystem2();

  void set_pool_size(int size);

  void update(double dt);

  void add_emitter(ParticleEmitter2 *emitter);
  void add_renderer(ParticleRenderer2 *renderer);
  void add_initializer(ParticleInitializer2 *init);
  void add_function(ParticleFunction2 *func);
  void add_force(ParticleForce2 *force);

  void add_child(ParticleSystem2 *child);

  void start(const NodePath &parent, double time = 0.0);
  void stop();

  void kill_particle(int n);
  bool birth_particles(int count);

public:
  typedef pvector<PT(ParticleInitializer2)> Initializers;
  Initializers _initializers;

  typedef pvector<PT(ParticleFunction2)> Functions;
  Functions _functions;

  typedef pvector<PT(ParticleEmitter2)> Emitters;
  Emitters _emitters;

  typedef pvector<PT(ParticleRenderer2)> Renderers;
  Renderers _renderers;

  typedef pvector<PT(ParticleForce2)> Forces;
  Forces _forces;

  // Resized to always contain _pool_size particles.
  typedef pvector<Particle> Particles;
  Particles _particles;

  double _elapsed;
  bool _running;
  int _pool_size;
  int _num_alive_particles;

  double _prev_dt;
  double _dt;

  pdeque<int> _free_particles;

  typedef pvector<PT(ParticleSystem2)> Children;
  Children _children;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ParticleSystem2",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "particleSystem2.I"

#endif // PARTICLESYSTEM2_H
