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
  P2_INIT_LifespanRandom::init_type();
  P2_INIT_PositionExplicit::init_type();
  P2_INIT_PositionBoxVolume::init_type();
  P2_INIT_PositionRectangleArea::init_type();
  P2_INIT_PositionSphereVolume::init_type();
  P2_INIT_PositionSphereSurface::init_type();
  P2_INIT_PositionCircleArea::init_type();
  P2_INIT_PositionCirclePerimeter::init_type();
  P2_INIT_PositionLineSegment::init_type();
  P2_INIT_PositionParametricCurve::init_type();
  P2_INIT_PositionCharacterJoints::init_type();
  P2_INIT_RotationRandom::init_type();

  ParticleEmitter2::init_type();
  BurstParticleEmitter::init_type();
  ContinuousParticleEmitter::init_type();

  ParticleFunction2::init_type();
  MotionParticleFunction::init_type();
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

  ParticleSystem2::init_type();
}
