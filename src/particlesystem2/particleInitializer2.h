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
 * Initializes particles to a random lifespawn within a given range.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_LifespanRandom : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_LifespanRandom, ParticleInitializer2);

PUBLISHED:
  P2_INIT_LifespanRandom(PN_stdfloat lifespan_min, PN_stdfloat lifespan_max);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PN_stdfloat _lifespan_min;
  PN_stdfloat _lifespan_max;
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
 * Initializes particles to a random position within the area of a given
 * 2-D rectangle.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionRectangleArea : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionRectangleArea, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionRectangleArea(const LPoint2 &a, const LPoint2 &b);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint2 _a;
  LPoint2 _b;
};

/**
 * Initializer that sets a particle's position to a random point within a
 * specified sphere.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionSphereVolume : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionSphereVolume, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionSphereVolume(const LPoint3 &center, PN_stdfloat radius);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _center;
  PN_stdfloat _radius;
};

/**
 * Initializes particle positions to a random point on the surface of a given
 * sphere.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionSphereSurface : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionSphereSurface, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionSphereSurface(const LPoint3 &center, PN_stdfloat radius_min, PN_stdfloat radius_max);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _center;
  PN_stdfloat _radius_min, _radius_max;
};

/**
 * Initializes particles to a random position within the area of a given
 * 2-D circle.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionCircleArea : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionCircleArea, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionCircleArea(const LPoint2 &center, PN_stdfloat radius);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint2 _center;
  PN_stdfloat _radius;
};

/**
 * Initializes particles to a random position on the perimeter of a given
 * 2-D circle.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_PositionCirclePerimeter : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_PositionCirclePerimeter, ParticleInitializer2);

PUBLISHED:
  P2_INIT_PositionCirclePerimeter(const LPoint2 &center, PN_stdfloat radius_min, PN_stdfloat radius_max);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint2 _center;
  PN_stdfloat _radius_min, _radius_max;
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
  PT(Character) _char;
  PN_stdfloat _radius;
};

/**
 * Initializes particle velocities to an explicit direction with potentially
 * randomized amplitude.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_VelocityExplicit : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_VelocityExplicit, ParticleInitializer2);

PUBLISHED:
  P2_INIT_VelocityExplicit(const LVector3 &dir, PN_stdfloat amplitude_min, PN_stdfloat amplitude_max);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LVector3 _vel;
  PN_stdfloat _amplitude_min;
  PN_stdfloat _amplitude_range;
};

/**
 * Initializes particle velocites to a random direction within a specified
 * angular cone and random amplitude.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_VelocityCone : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_VelocityCone, ParticleInitializer2);

PUBLISHED:
  P2_INIT_VelocityCone(const LVecBase3 &min_hpr, const LVecBase3 &max_hpr,
                       PN_stdfloat min_amplitude, PN_stdfloat max_amplitude);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LVecBase3 _min_hpr;
  LVecBase3 _max_hpr;

  PN_stdfloat _min_amplitude;
  PN_stdfloat _max_amplitude;
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
                          PN_stdfloat min_amplitude, PN_stdfloat max_amplitude);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  LPoint3 _point;
  PN_stdfloat _min_amplitude;
  PN_stdfloat _max_amplitude;
};

/**
 * Initializes particles to a random rotation and rotational velocity.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 P2_INIT_RotationRandom : public ParticleInitializer2 {
  DECLARE_CLASS(P2_INIT_RotationRandom, ParticleInitializer2);

PUBLISHED:
  P2_INIT_RotationRandom(PN_stdfloat min_rot, PN_stdfloat max_rot,
                         PN_stdfloat min_rot_speed, PN_stdfloat max_rot_speed);

public:
  virtual void init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) override;

private:
  PN_stdfloat _min_rot, _max_rot;
  PN_stdfloat _min_rot_speed, _max_rot_speed;
};

#include "particleInitializer2.I"

#endif // PARTICLEINITIALIZER2_H
