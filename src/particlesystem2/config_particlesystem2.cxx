/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_particlesystem2.cxx
 * @author brian
 * @date 2022-04-02
 */

#include "config_particlesystem2.h"
#include "particleFunction2.h"
#include "particleEmitter2.h"
#include "particleSystem2.h"
#include "particleInitializer2.h"
#include "particleRenderer2.h"
#include "particleForce2.h"
#include "particleConstraint2.h"

ConfigureDef(config_particlesystem2);
ConfigureFn(config_particlesystem2) {
  init_libparticlesystem2();
}

NotifyCategoryDef(particlesystem2, "");

/**
 *
 */
void
init_libparticlesystem2() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  ParticleInitializer2::init_type();
  P2_INIT_LifespanRandomRange::init_type();
  P2_INIT_PositionExplicit::init_type();
  P2_INIT_PositionBoxVolume::init_type();
  P2_INIT_PositionSphereVolume::init_type();
  P2_INIT_PositionLineSegment::init_type();
  P2_INIT_PositionParametricCurve::init_type();
  P2_INIT_PositionCharacterJoints::init_type();
  P2_INIT_VelocityExplicit::init_type();
  P2_INIT_VelocityCone::init_type();
  P2_INIT_VelocityRadiate::init_type();
  P2_INIT_RotationRandomRange::init_type();
  P2_INIT_RotationVelocityRandomRange::init_type();
  P2_INIT_ScaleRandomRange::init_type();
  P2_INIT_ColorRandomRange::init_type();
  P2_INIT_AlphaRandomRange::init_type();
  P2_INIT_RemapAttribute::init_type();
  P2_INIT_PositionModelHitBoxes::init_type();
  P2_INIT_AnimationIndexRandom::init_type();
  P2_INIT_AnimationFPSRandom::init_type();

  ParticleEmitter2::init_type();
  BurstParticleEmitter::init_type();
  ContinuousParticleEmitter::init_type();

  ParticleFunction2::init_type();
  LinearMotionParticleFunction::init_type();
  AngularMotionParticleFunction::init_type();
  LifespanKillerParticleFunction::init_type();
  LerpParticleFunction::init_type();
  VelocityJitterParticleFunction::init_type();
  BounceParticleFunction::init_type();

  ParticleRenderer2::init_type();
  SpriteParticleRenderer2::init_type();

  ParticleForce2::init_type();
  VectorParticleForce::init_type();
  CylinderVortexParticleForce::init_type();
  JitterParticleForce::init_type();
  AttractParticleForce::init_type();
  FrictionParticleForce::init_type();

  ParticleConstraint2::init_type();
  PathParticleConstraint::init_type();

  ParticleSystem2::init_type();

  P2_INIT_LifespanRandomRange::register_with_read_factory();
  P2_INIT_PositionExplicit::register_with_read_factory();
  P2_INIT_PositionBoxVolume::register_with_read_factory();
  P2_INIT_PositionSphereVolume::register_with_read_factory();
  P2_INIT_PositionLineSegment::register_with_read_factory();
  P2_INIT_PositionParametricCurve::register_with_read_factory();
  P2_INIT_VelocityExplicit::register_with_read_factory();
  P2_INIT_VelocityCone::register_with_read_factory();
  P2_INIT_VelocityRadiate::register_with_read_factory();
  P2_INIT_RotationRandomRange::register_with_read_factory();
  P2_INIT_RotationVelocityRandomRange::register_with_read_factory();
  P2_INIT_ScaleRandomRange::register_with_read_factory();
  P2_INIT_ColorRandomRange::register_with_read_factory();
  P2_INIT_AlphaRandomRange::register_with_read_factory();
  P2_INIT_RemapAttribute::register_with_read_factory();
  P2_INIT_PositionModelHitBoxes::register_with_read_factory();
  P2_INIT_AnimationIndexRandom::register_with_read_factory();
  P2_INIT_AnimationFPSRandom::register_with_read_factory();

  LinearMotionParticleFunction::register_with_read_factory();
  AngularMotionParticleFunction::register_with_read_factory();
  LifespanKillerParticleFunction::register_with_read_factory();
  LerpParticleFunction::register_with_read_factory();
  VelocityJitterParticleFunction::register_with_read_factory();
  BounceParticleFunction::register_with_read_factory();

  BurstParticleEmitter::register_with_read_factory();
  ContinuousParticleEmitter::register_with_read_factory();

  SpriteParticleRenderer2::register_with_read_factory();

  PathParticleConstraint::register_with_read_factory();

  VectorParticleForce::register_with_read_factory();
  CylinderVortexParticleForce::register_with_read_factory();
  JitterParticleForce::register_with_read_factory();
  AttractParticleForce::register_with_read_factory();
  FrictionParticleForce::register_with_read_factory();

  ParticleSystem2::register_with_read_factory();
}
