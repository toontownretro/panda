/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleInitializer2.h
 * @author brian
 * @date 2022-04-04
 */

#ifndef PARTICLEINITIALIZER2_H
#define PARTICLEINITIALIZER2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "parametricCurve.h"
#include "character.h"
#include "pt_Character.h"

class ParticleSystem2;

/**
 * Initializers are responsible for setting up the initial properties of
 * particles on spawn, such as position, velocity, color, etc.
 *
 * Each initializer of a particle system is invoked once for every particle
 * that is spawned.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleInitializer2 : public TypedWritableReferenceCount {
  DECLARE_CLASS(ParticleInitializer2, TypedWritableReferenceCount);

PUBLISHED:
  ParticleInitializer2() = default;

public:
  /**
   *
   */
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system)=0;
};

/**
 * Initializes particles to a random lifespan within a given range.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_LifespanRandomRange : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_LifespanRandomRange, ParticleInitializer2);

PUBLISHED:
  P2_INIT_LifespanRandomRange(PN_stdfloat lifespan_min, PN_stdfloat lifespan_max,
                              PN_stdfloat exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PN_stdfloat _lifespan_min;
  PN_stdfloat _lifespan_range;
  PN_stdfloat _lifespan_exponent;
};

/**
 * Initializes particle positions to an explicit point in space.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionExplicit : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionExplicit, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionExplicit(const LPoint3 &point);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _point;
};

/**
 * Initializer that sets a particle's position to a random point within the
 * volume of a specified box.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionBoxVolume : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionBoxVolume, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionBoxVolume(const LPoint3 &mins, const LPoint3 &maxs);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _mins;
  LPoint3 _maxs;
};

/**
 * Initializer that sets a particle's position to a random point within a
 * specified sphere.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionSphereVolume : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionSphereVolume, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionSphereVolume(const LPoint3 &center, PN_stdfloat radius_min, PN_stdfloat radius_max,
                               const LVecBase3 &bias = LVecBase3(1.0f),
                               const LVecBase3 &scale = LVecBase3(1.0f),
                               const LVecBase3i &absolute_value = LVecBase3i(0));

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _center;
  PN_stdfloat _radius_min;
  PN_stdfloat _radius_range;
  LVecBase3 _bias;
  LVecBase3 _scale;
  LVecBase3i _absolute_value;
};

/**
 * Initializes particle positions to a random point along a single line
 * segment.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionLineSegment : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionLineSegment, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionLineSegment(const LPoint3 &a, const LPoint3 &b);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _a;
  LPoint3 _b;
};

/**
 * Initializes particle positions to a random point along a user-defined
 * ParametricCurve type.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionParametricCurve : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionParametricCurve, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionParametricCurve(ParametricCurve *curve);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PT(ParametricCurve) _curve;
};

/**
 * Initializes particle positions to a random point around a joint of a
 * character.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionCharacterJoints : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionCharacterJoints, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionCharacterJoints(Character *character, PN_stdfloat radius);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PT_Character _char;
  PN_stdfloat _radius;
};

/**
 * Initializes particle velocities to an explicit direction with potentially
 * randomized amplitude.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_VelocityExplicit : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_VelocityExplicit, ParticleInitializer2);

PUBLISHED:
  P2_INIT_VelocityExplicit(const LVector3 &dir, PN_stdfloat amplitude_min, PN_stdfloat amplitude_max,
                           PN_stdfloat amplitude_exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LVector3 _vel;
  PN_stdfloat _amplitude_min;
  PN_stdfloat _amplitude_range;
  PN_stdfloat _amplitude_exponent;
};

/**
 * Initializes particle velocites to a random direction within a specified
 * angular cone and random amplitude.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_VelocityCone : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_VelocityCone, ParticleInitializer2);

PUBLISHED:
  P2_INIT_VelocityCone(const LVecBase3 &min_hpr, const LVecBase3 &max_hpr,
                       PN_stdfloat min_amplitude, PN_stdfloat max_amplitude,
                       PN_stdfloat amplitude_exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LVecBase3 _min_hpr;
  LVecBase3 _max_hpr;

  PN_stdfloat _min_amplitude;
  PN_stdfloat _amplitude_range;
  PN_stdfloat _amplitude_exponent;
};

/**
 * Initializes particle linear velocities to radiate from a given point in
 * space.
 *
 * Velocity vector is vector from radiate point to particle position, scaled
 * by a random amplitude.  Initializer should run after particle position has
 * been initialized.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_VelocityRadiate : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_VelocityRadiate, ParticleInitializer2);

PUBLISHED:
  P2_INIT_VelocityRadiate(const LPoint3 &point,
                          PN_stdfloat min_amplitude, PN_stdfloat max_amplitude,
                          PN_stdfloat amplitude_exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _point;
  PN_stdfloat _min_amplitude;
  PN_stdfloat _amplitude_range;
  PN_stdfloat _amplitude_exponent;
};

/**
 * Initializes particles to a random rotation.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_RotationRandomRange : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_RotationRandomRange, ParticleInitializer2);

PUBLISHED:
  P2_INIT_RotationRandomRange(PN_stdfloat base, PN_stdfloat offset_min, PN_stdfloat offset_max,
                         PN_stdfloat offset_exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PN_stdfloat _rot_base;
  PN_stdfloat _offset_min, _offset_range, _offset_exponent;
};

/**
 * Initializes particles to a random rotational velocity.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_RotationVelocityRandomRange : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_RotationVelocityRandomRange, ParticleInitializer2);

PUBLISHED:
  P2_INIT_RotationVelocityRandomRange(PN_stdfloat speed_min, PN_stdfloat speed_max,
                                 PN_stdfloat speed_exponent = 1.0f, bool random_flip = false,
                                 PN_stdfloat random_flip_chance = 0.5f,
                                 PN_stdfloat random_flip_exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PN_stdfloat _vel_min, _vel_range, _vel_exponent;

  // If true, chosen rotational velocity has a random chance of being
  // flipped to spin in opposite direction.
  bool _random_flip;
  PN_stdfloat _random_flip_chance, _random_flip_exponent;
};

/**
 * Initializes particles to a random scale within a given range.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_ScaleRandomRange : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_ScaleRandomRange, ParticleInitializer2);

PUBLISHED:
  P2_INIT_ScaleRandomRange(const LVecBase3 &scale_min, const LVecBase3 &scale_max,
                           bool componentwise = true,
                           PN_stdfloat scale_exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LVecBase3 _scale_min, _scale_range;
  PN_stdfloat _scale_exponent;
  bool _componentwise;
};

/**
 * Initializes particles to a random RGB color within a given range.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_ColorRandomRange : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_ColorRandomRange, ParticleInitializer2);

PUBLISHED:
  P2_INIT_ColorRandomRange(const LVecBase3 &color1, const LVecBase3 &color2, bool componentwise = false,
                           PN_stdfloat exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LVecBase3 _color_min, _color_range;
  PN_stdfloat _exponent;
  bool _componentwise;
};

/**
 * Initializes particles to a random alpha value within a given range.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_AlphaRandomRange : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_AlphaRandomRange, ParticleInitializer2);

PUBLISHED:
  P2_INIT_AlphaRandomRange(PN_stdfloat alpha_min, PN_stdfloat alpha_max, PN_stdfloat exponent = 1.0f);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PN_stdfloat _alpha_min, _alpha_range, _alpha_exponent;
};

#include "particleInitializer2.I"

#endif // PARTICLEINITIALIZER2_H
