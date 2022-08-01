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
  // Defines which axes the force should apply to.
  enum AxisMask {
    AM_x = 1,
    AM_y = 2,
    AM_z = 4,
    AM_all = (AM_x | AM_y | AM_z),
  };

  ParticleForce2();

  void set_axis_mask(unsigned int mask);

  INLINE LVector3 apply_axis_mask(const LVector3 &vec);

public:
  /**
   * Accumulates the force onto all particles in the system.
   */
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system)=0;

protected:
  unsigned int _axis_mask;
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
  enum AxisMode {
    AM_explicit, // Explicit axis
    AM_input, // Axis taken from rotation of input
    AM_vec_between_inputs, // Axis set to vector between positions of two inputs
  };
  CylinderVortexParticleForce(PN_stdfloat coef = 1.0f, const LVector3 &axis = LVector3::up(),
                              const LPoint3 &center = LPoint3(0.0f));

  void set_local_axis(bool flag);
  void set_input0(int input);
  void set_input1(int input);
  void set_mode(AxisMode mode);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  AxisMode _mode;

  int _input0, _input1;

  LVector3 _axis;
  bool _local_axis;

  PN_stdfloat _coef;
  LPoint3 _center;
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
  AttractParticleForce(int input, const LPoint3 &point, PN_stdfloat falloff, PN_stdfloat amplitude,
                       PN_stdfloat radius = -1.0f);

  void set_point(const LPoint3 &point);

public:
  virtual void accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) override;

private:
  int _input;
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
