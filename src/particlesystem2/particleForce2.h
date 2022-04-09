/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleForce2.h
 * @author brian
 * @date 2022-04-06
 */

#ifndef PARTICLEFORCE2_H
#define PARTICLEFORCE2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "luse.h"

class ParticleSystem2;

/**
 * Base class for a physical force that is applied to a particle system,
 * such as gravity.
 *
 * The force is applied to all particles in the system, but can be configured
 * to only apply to particles on a certain range of their lifespan.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleForce2 : public TypedWritableReferenceCount {
  DECLARE_CLASS(ParticleForce2, TypedWritableReferenceCount);
PUBLISHED:
  ParticleForce2() = default;

public:
  /**
   * Accumulates the force onto all particles in the system.
   */
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system)=0;
};

/**
 * Identical to LinearVectorForce from the old particle system.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 VectorParticleForce : public ParticleForce2 {
  DECLARE_CLASS(VectorParticleForce, ParticleForce2);

PUBLISHED:
  VectorParticleForce(const LVector3 &vector);

  void set_vector(const LVector3 &vector);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  LVector3 _force;
};

/**
 * Idential to LinearCylinderVortexForce from the old particle system.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 CylinderVortexParticleForce : public ParticleForce2 {
  DECLARE_CLASS(CylinderVortexParticleForce, ParticleForce2);

PUBLISHED:
  CylinderVortexParticleForce(PN_stdfloat radius, PN_stdfloat length,
                              PN_stdfloat coef, const LVecBase3 &hpr, const LPoint3 &center);

  void set_transform(const LVecBase3 &hpr, const LPoint3 &center);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  PN_stdfloat _radius, _length, _coef;
  // Transform world-space particle position into force space.
  LMatrix4 _transform;
  // Transform force space into world-space.
  LMatrix4 _inv_transform;
};

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 JitterParticleForce : public ParticleForce2 {
  DECLARE_CLASS(JitterParticleForce, ParticleForce2);

PUBLISHED:
  JitterParticleForce(PN_stdfloat amplitude, PN_stdfloat start = 0.0f, PN_stdfloat end = 1.0f);

  void set_amplitude(PN_stdfloat a);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  PN_stdfloat _amplitude;
  PN_stdfloat _start, _end;
};

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 AttractParticleForce : public ParticleForce2 {
  DECLARE_CLASS(AttractParticleForce, ParticleForce2);

PUBLISHED:
  AttractParticleForce(const LPoint3 &point, PN_stdfloat falloff, PN_stdfloat amplitude,
                       PN_stdfloat radius = -1.0f);

  void set_point(const LPoint3 &point);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  // TODO: Get point from input.
  LPoint3 _point;
  PN_stdfloat _amplitude;
  PN_stdfloat _falloff;
  PN_stdfloat _radius;
};

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 FrictionParticleForce : public ParticleForce2 {
  DECLARE_CLASS(FrictionParticleForce, ParticleForce2);

PUBLISHED:
  FrictionParticleForce(PN_stdfloat coefficient);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  PN_stdfloat _coef;
};

#include "particleForce2.I"

#endif // PARTICLEFORCE2_H
