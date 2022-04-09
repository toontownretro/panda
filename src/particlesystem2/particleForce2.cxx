/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleForce2.cxx
 * @author brian
 * @date 2022-04-06
 */

#include "particleForce2.h"
#include "particleSystem2.h"
#include "p2_utils.h"

IMPLEMENT_CLASS(ParticleForce2);
IMPLEMENT_CLASS(VectorParticleForce);
IMPLEMENT_CLASS(CylinderVortexParticleForce);
IMPLEMENT_CLASS(JitterParticleForce);
IMPLEMENT_CLASS(AttractParticleForce);
IMPLEMENT_CLASS(FrictionParticleForce);

/**
 *
 */
VectorParticleForce::
VectorParticleForce(const LVector3 &force) :
  _force(force)
{
}

/**
 *
 */
void VectorParticleForce::
set_vector(const LVector3 &force) {
  _force = force;
}

/**
 *
 */
void VectorParticleForce::
accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }
    *accum += _force * strength;
    ++accum;
  }
}

/**
 *
 */
CylinderVortexParticleForce::
CylinderVortexParticleForce(PN_stdfloat radius, PN_stdfloat length, PN_stdfloat coef,
                            const LVecBase3 &hpr, const LPoint3 &center) :
  _radius(radius),
  _length(length),
  _coef(coef)
{
  set_transform(hpr, center);
}

/**
 * Sets the rotation and center point of the cylinder vortex.
 */
void CylinderVortexParticleForce::
set_transform(const LVecBase3 &hpr, const LPoint3 &center) {
  compose_matrix(_transform, LVecBase3(1.0f), hpr, center);
  _inv_transform.invert_from(_transform);
}

/**
 *
 */
void CylinderVortexParticleForce::
accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    LPoint3 local = _transform.xform_point(p._pos);

    // Clip to cylinder length.
    if (local[2] < -_length || local[2] > _length) {
      ++accum;
      continue;
    }

    // Clip to cylinder radius.
    PN_stdfloat x_squared = local[0] * local[0];
    PN_stdfloat y_squared = local[1] * local[1];
    PN_stdfloat dist_squared = x_squared + y_squared;
    PN_stdfloat radius_squared = _radius * _radius;
    if (dist_squared > radius_squared) {
      ++accum;
      continue;
    }
    if (IS_NEARLY_ZERO(dist_squared)) {
      ++accum;
      continue;
    }

    PN_stdfloat r = csqrt(dist_squared);
    if (IS_NEARLY_ZERO(r)) {
      ++accum;
      continue;
    }

    LVector3 tangential = local;
    tangential[2] = 0.0f;
    tangential.normalize();
    tangential = tangential.cross(LVector3(0.0f, 0.0f, 1.0f));

    LVector3 centripetal = -local;
    centripetal[2] = 0.0f;
    centripetal.normalize();

    LVector3 combined = tangential + centripetal;
    combined.normalize();

    centripetal = combined * _coef * strength * p._velocity.length();

    *accum += _inv_transform.xform_vec(centripetal);
    ++accum;
  }
}

/**
 *
 */
JitterParticleForce::
JitterParticleForce(PN_stdfloat a, PN_stdfloat start, PN_stdfloat end) :
  _amplitude(a),
  _start(start),
  _end(end)
{
}

/**
 *
 */
void JitterParticleForce::
set_amplitude(PN_stdfloat a) {
  _amplitude = a;
}

/**
 *
 */
void JitterParticleForce::
accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }
    PN_stdfloat elapsed = system->_elapsed - p._spawn_time;
    PN_stdfloat frac = elapsed / p._duration;
    if (frac < _start || frac > _end) {
      ++accum;
      continue;
    }

    *accum += p2_random_unit_vector() * strength * _amplitude;
    ++accum;
  }
}

/**
 *
 */
AttractParticleForce::
AttractParticleForce(const LPoint3 &point, PN_stdfloat falloff, PN_stdfloat amplitude,
                     PN_stdfloat radius) :
  _point(point),
  _falloff(falloff),
  _amplitude(amplitude),
  _radius(radius)
{
}

/**
 *
 */
void AttractParticleForce::
set_point(const LPoint3 &point) {
  _point = point;
}

/**
 *
 */
void AttractParticleForce::
accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    //if (_radius > 0.0f) {
    //  if ((p._pos - _point).length_squared() > (_radius * _radius)) {
    //    ++accum;
    //    continue;
    //  }
    //}

    // Attract to force point.
    LVector3 vec = p._pos - _point;
    PN_stdfloat len = vec.length();
    if (IS_NEARLY_ZERO(len)) {
      accum++;
      continue;
    }
    vec /= len;
    vec *= -_amplitude * strength;
    vec /= std::pow(len, _falloff);
    *accum += vec;
    ++accum;
  }
}

/**
 *
 */
FrictionParticleForce::
FrictionParticleForce(PN_stdfloat coef) :
  _coef(coef)
{
}

/**
 *
 */
void FrictionParticleForce::
accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    *accum -= p._velocity * _coef;
    ++accum;
  }
}
