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
#include "nodePath.h"
#include "transformState.h"

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
  INLINE int get_pool_size() const;
  MAKE_PROPERTY(pool_size, get_pool_size, set_pool_size);

  void update(double dt);

  void add_emitter(ParticleEmitter2 *emitter);
  INLINE int get_num_emitters() const;
  INLINE ParticleEmitter2 *get_emitter(int n) const;
  MAKE_SEQ(get_emitters, get_num_emitters, get_emitter);
  MAKE_SEQ_PROPERTY(emitters, get_num_emitters, get_emitter);

  void add_renderer(ParticleRenderer2 *renderer);
  INLINE int get_num_renderers() const;
  INLINE ParticleRenderer2 *get_renderer(int n) const;
  MAKE_SEQ(get_renderers, get_num_renderers, get_renderer);
  MAKE_SEQ_PROPERTY(renderers, get_num_renderers, get_renderer);

  void add_initializer(ParticleInitializer2 *init);
  INLINE int get_num_initializers() const;
  INLINE ParticleInitializer2 *get_initializer(int n) const;
  MAKE_SEQ(get_initializers, get_num_initializers, get_initializer);
  MAKE_SEQ_PROPERTY(initializers, get_num_initializers, get_initializer);

  void add_function(ParticleFunction2 *func);
  INLINE int get_num_functions() const;
  INLINE ParticleFunction2 *get_function(int n) const;
  MAKE_SEQ(get_functions, get_num_functions, get_function);
  MAKE_SEQ_PROPERTY(functions, get_num_functions, get_function);

  void add_force(ParticleForce2 *force);
  INLINE int get_num_forces() const;
  INLINE ParticleForce2 *get_force(int n) const;
  MAKE_SEQ(get_forces, get_num_forces, get_force);
  MAKE_SEQ_PROPERTY(forces, get_num_forces, get_force);

  void add_child(ParticleSystem2 *child);
  INLINE int get_num_children() const;
  INLINE ParticleSystem2 *get_child(int n) const;
  MAKE_SEQ(get_children, get_num_children, get_child);
  MAKE_SEQ_PROPERTY(children, get_num_children, get_child);

  void add_input(const NodePath &input);
  void set_input(int n, const NodePath &input);
  INLINE int get_num_inputs() const;
  INLINE const NodePath &get_input(int n) const;
  INLINE const TransformState *get_input_value(int n) const;
  MAKE_SEQ(get_inputs, get_num_inputs, get_input);
  MAKE_SEQ_PROPERTY(inputs, get_num_inputs, get_input);
  MAKE_SEQ(get_input_values, get_num_inputs, get_input_value);
  MAKE_SEQ_PROPERTY(input_values, get_num_inputs, get_input_value);

  void start(const NodePath &parent, double time = 0.0);
  void stop();

  INLINE const NodePath &get_parent_node() const;
  MAKE_PROPERTY(parent_node, get_parent_node);

  INLINE bool is_running() const;
  MAKE_PROPERTY(running, is_running);

  INLINE double get_elapsed_time() const;
  MAKE_PROPERTY(elapsed_time, get_elapsed_time);

  INLINE int get_num_alive_particles() const;
  MAKE_PROPERTY(num_alive_particles, get_num_alive_particles);

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

  // NodePaths whose transforms can be used to influence the behavior of
  // the particle system.  By convention, input 0 defines the emission
  // coordinate space.  All other inputs can be interpreted as needed on
  // a per-initializer/function basis.
  typedef pvector<NodePath> Inputs;
  Inputs _inputs;
  // Pre-fetched system-space transform of each input node.  Updated
  // at the beginning of each system update.
  typedef pvector<CPT(TransformState)> InputValues;
  InputValues _input_values;

  // Node that the particle system is parented to.
  // Normally, this is render, or the root node of the scene graph.
  // Particle systems normally don't inherit any transforms, except for
  // initialization (emission relative to another node).
  NodePath _parent;

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
