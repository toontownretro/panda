/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleEmitter2.h
 * @author brian
 * @date 2022-04-04
 */

#ifndef PARTICLEEMITTER2_H
#define PARTICLEEMITTER2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"

/**
 * Emitters are responsible for spawning new particles in a system.
 * Each emitter type spawns particles in a different manner.
 *
 * Emitters simply spawn new particles, they do not set up properties
 * or perform any operations on the particles themselves.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleEmitter2 : public TypedWritableReferenceCount {
PUBLISHED:
  ParticleEmitter2() = default;

public:
  /**
   * Called on each update step of the particle system.  Returns the number of
   * new particles to spawn.  Derived emitters are responsible for determining
   * when to spawn new particles and how many.
   *
   * `time` is the number of seconds elapsed since the particle system began.
   */
  virtual int update(double time)=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ParticleEmitter2",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 * A particle emitter that emits N particles in a single burst.  This emitter
 * should be used for one-off particle effects such as explosions.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 BurstParticleEmitter : public ParticleEmitter2 {
PUBLISHED:
  BurstParticleEmitter();

public:
  virtual int update(double time) override;

private:
  PN_stdfloat _start_time;
  int _litter_min;
  int _litter_max;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleEmitter2::init_type();
    register_type(_type_handle, "BurstParticleEmitter",
                  ParticleEmitter2::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 * A particle emitter that continuously emits X particles every Y seconds.
 * This emitter should be used for continuous particle effects such as rain,
 * smoke, etc.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ContinuousParticleEmitter : public ParticleEmitter2 {
PUBLISHED:
  ContinuousParticleEmitter();

  void set_emission_rate(PN_stdfloat particles_per_second);
  void set_interval_and_litter_size(PN_stdfloat interval_min, PN_stdfloat interval_max,
                                    int litter_min, int litter_max);

  void set_start_time(PN_stdfloat time);
  void set_duration(PN_stdfloat duration);

public:
  virtual int update(double time) override;

private:
  // System-relative time to start emitting particles.
  PN_stdfloat _start_time;

  // Randomized time range between particle births.
  PN_stdfloat _interval_min;
  PN_stdfloat _interval_max;

  // Randomized range of number of particles to birth
  // each emission interval.
  int _litter_min;
  int _litter_max;

  // How long emitter should be active for.
  PN_stdfloat _duration;

  PN_stdfloat _next_litter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleEmitter2::init_type();
    register_type(_type_handle, "ContinuousParticleEmitter",
                  ParticleEmitter2::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "particleEmitter2.I"

#endif // PARTICLEEMITTER2_H
