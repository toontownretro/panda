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
#include "factoryParams.h"

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

  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

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

  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);
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

  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

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
 * Function that kills particles that go under a velocity threshold.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 VelocityKillerParticleFunction : public ParticleFunction2 {
PUBLISHED:
  VelocityKillerParticleFunction(PN_stdfloat threshold);

  INLINE void set_threshold(PN_stdfloat threshold) { _threshold = threshold; }
  INLINE PN_stdfloat get_threshold() const { return _threshold; }

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParticleFunction2::init_type();
    register_type(_type_handle, "VelocityKillerParticleFunction",
                  ParticleFunction2::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  PN_stdfloat _threshold;
};

class EXPCL_PANDA_PARTICLESYSTEM2 ParticleLerpSegment {
PUBLISHED:
  enum LerpType {
    LT_constant,
    LT_linear,
    LT_exponential,
    LT_stepwave,
    LT_sinusoid,
  };

  INLINE ParticleLerpSegment();
  ~ParticleLerpSegment() = default;

  INLINE void set_range(float start, float end);
  INLINE void set_start(float start);
  INLINE float get_start() const;
  INLINE void set_end(float end);
  INLINE float get_end() const;
  MAKE_PROPERTY(start, get_start, set_start);
  MAKE_PROPERTY(end, get_end, set_end);

  INLINE void set_type(LerpType type);
  INLINE LerpType get_type() const;
  MAKE_PROPERTY(type, get_type, set_type);

  INLINE void set_start_value(const LVecBase3 &value);
  INLINE const LVecBase3 &get_start_value() const;
  MAKE_PROPERTY(start_value, get_start_value, set_start_value);

  INLINE void set_end_value(const LVecBase3 &value);
  INLINE const LVecBase3 &get_end_value() const;
  MAKE_PROPERTY(end_value, get_end_value, set_end_value);

  INLINE void set_scale_on_initial(bool flag);
  INLINE bool is_scale_on_initial() const;
  MAKE_PROPERTY(scale_on_initial, is_scale_on_initial, set_scale_on_initial);

  INLINE void set_start_is_initial(bool flag);
  INLINE bool get_start_is_initial() const;
  MAKE_PROPERTY(start_is_initial, get_start_is_initial, set_start_is_initial);

  INLINE void set_end_is_initial(bool flag);
  INLINE bool get_end_is_initial() const;
  MAKE_PROPERTY(end_is_initial, get_end_is_initial, set_end_is_initial);

  // Only for LT_exponential.
  INLINE void set_exponent(float exp);
  INLINE float get_exponent() const;
  MAKE_PROPERTY(exponent, get_exponent, set_exponent);

  // Only for LT_sinusoid.
  INLINE void set_period(float period);
  INLINE float get_period() const;
  MAKE_PROPERTY(period, get_period, set_period);

  // Only for LT_stepwave.
  INLINE void set_step_start_width(float width);
  INLINE float get_step_start_width() const;
  MAKE_PROPERTY(step_start_width, get_step_start_width, set_step_start_width);
  INLINE void set_step_end_width(float width);
  INLINE float get_step_end_width() const;
  MAKE_PROPERTY(step_end_width, get_step_end_width, set_step_end_width);

public:
  PN_stdfloat _start_frac;
  PN_stdfloat _end_frac;

  LerpType _type;

  LVecBase3 _start_value;
  LVecBase3 _end_value;

  // If true, start_value and end_value are scales on the initial particle
  // attribute value, rather than concrete values for the attribute.
  bool _scale_on_initial;

  bool _start_is_initial;
  bool _end_is_initial;

  PN_stdfloat _func_data[2];
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

  LerpParticleFunction(Component component);

  void add_segment(const ParticleLerpSegment &segment);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  LerpParticleFunction() = default;

private:
  Component _component;
  typedef pvector<ParticleLerpSegment> LerpSegments;
  LerpSegments _segments;
};

/**
 * Function that applies random instantaneous velocity changes to particles.
 * See JitterParticleForce for random forces instead of velocity changes.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 VelocityJitterParticleFunction : public ParticleFunction2 {
  DECLARE_CLASS(VelocityJitterParticleFunction, ParticleFunction2);

PUBLISHED:
  VelocityJitterParticleFunction(PN_stdfloat amplitude_min, PN_stdfloat amplitude_max,
                                 const LVecBase3 &scale = LVecBase3(1.0f), PN_stdfloat start = 0.0, PN_stdfloat end = 1.0);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  VelocityJitterParticleFunction() = default;

private:
  PN_stdfloat _amplitude_min;
  PN_stdfloat _amplitude_range;
  PN_stdfloat _start, _end;
  LVecBase3 _scale;
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

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  BounceParticleFunction() = default;

private:
  LPlane _plane;
  PN_stdfloat _bounciness;
};

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 FollowInputParticleFunction : public ParticleFunction2 {
  DECLARE_CLASS(FollowInputParticleFunction, ParticleFunction2);

PUBLISHED:
  FollowInputParticleFunction(int input);

public:
  virtual void update(double time, double dt, ParticleSystem2 *system) override;

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

protected:
  FollowInputParticleFunction() = default;

private:
  int _input;
};

#include "particleFunction2.I"

#endif // PARTICLEFUNCTION2_H
