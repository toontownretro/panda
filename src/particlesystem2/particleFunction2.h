/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleFunction2.h
 * @author brian
 * @date 2022-04-04
 */

#ifndef PARTICLEFUNCTION2_H
#define PARTICLEFUNCTION2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "plane.h"

class ParticleSystem2;

/**
 * Functions are responsible for carrying out operations that change the
 * properties of particles over time.  They define how each particle in a
 * system behaves over its lifespan.
 *
 * Examples of functions are color changes and force application (gravity,
 * etc).
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleFunction2 : public TypedWritableReferenceCount {
PUBLISHED:
  ParticleFunction2() = default;

public:
  virtual void update(double time, double dt, ParticleSystem2 *system)=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "ParticleFunction2",
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
 * A function that applies linear motion to the particle according to its
 * velocity vector and applied forces.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 LinearMotionParticleFunction : public ParticleFunction2 {
PUBLISHED:
  LinearMotionParticleFunction(PN_stdfloat drag = 0.0f);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

private:
  PN_stdfloat _drag;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleFunction2::init_type();
    register_type(_type_handle, "MotionParticleFunction",
                  ParticleFunction2::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 * Applies angular velocity to the rotation of particles so they rotate over
 * time.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 AngularMotionParticleFunction : public ParticleFunction2 {
  DECLARE_CLASS(AngularMotionParticleFunction, ParticleFunction2);

PUBLISHED:
  AngularMotionParticleFunction() = default;

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;
};

/**
 * Function that kills particles whose lifetime has exceeded their chosen
 * duration.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 LifespanKillerParticleFunction : public ParticleFunction2 {
PUBLISHED:
  LifespanKillerParticleFunction() = default;

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleFunction2::init_type();
    register_type(_type_handle, "LifespanKillerParticleFunction",
                  ParticleFunction2::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 * Lerps a particle component over its lifetime with interpolation segments
 * and functions.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 LerpParticleFunction : public ParticleFunction2 {
  DECLARE_CLASS(LerpParticleFunction, ParticleFunction2);

PUBLISHED:
  enum Component {
    // Particle attributes that can be lerped.
    C_rgb,
    C_alpha,
    C_scale,
    C_rotation,
  };

  enum LerpType {
    LT_constant,
    LT_linear,
    LT_exponential,
    LT_stepwave,
    LT_sinusoid,
  };

  LerpParticleFunction(Component component);

  void add_constant_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &value);
  void add_linear_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                          const LVecBase3 &end_value);
  void add_exponential_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                               const LVecBase3 &end_value, PN_stdfloat exponent = 1.0f);
  void add_stepwave_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                            const LVecBase3 &end_value, PN_stdfloat start_width = 0.5f, PN_stdfloat end_width = 0.5f);
  void add_sinusoid_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                            const LVecBase3 &end_value, PN_stdfloat period = 1.0f);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

private:
  Component _component;

  class LerpSegment {
  public:
    PN_stdfloat _start_frac;
    PN_stdfloat _end_frac;

    LerpType _type;

    LVecBase3 _start_value;
    LVecBase3 _end_value;

    bool _start_is_initial;
    bool _end_is_initial;

    PN_stdfloat _func_data[2];
  };
  typedef pvector<LerpSegment> LerpSegments;
  LerpSegments _segments;
};

/**
 * Function that applies random instantaneous velocity changes to particles.
 * See JitterParticleForce for random forces instead of velocity changes.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 VelocityJitterParticleFunction : public ParticleFunction2 {
  DECLARE_CLASS(VelocityJitterParticleFunction, ParticleFunction2);

PUBLISHED:
  VelocityJitterParticleFunction(PN_stdfloat amplitude_min, PN_stdfloat amplitude_max);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

private:
  PN_stdfloat _amplitude_min;
  PN_stdfloat _amplitude_range;
};

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 BounceParticleFunction : public ParticleFunction2 {
  DECLARE_CLASS(BounceParticleFunction, ParticleFunction2);

PUBLISHED:
  BounceParticleFunction(const LPlane &plane, PN_stdfloat bounciness);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

private:
  LPlane _plane;
  PN_stdfloat _bounciness;
};

#include "particleFunction2.I"

#endif // PARTICLEFUNCTION2_H
