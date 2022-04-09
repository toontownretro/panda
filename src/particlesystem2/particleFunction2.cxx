/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleFunction2.cxx
 * @author brian
 * @date 2022-04-04
 */

#include "particleFunction2.h"
#include "particleSystem2.h"
#include "mathutil_misc.h"
#include "p2_utils.h"

TypeHandle ParticleFunction2::_type_handle;
TypeHandle MotionParticleFunction::_type_handle;
TypeHandle LifespanKillerParticleFunction::_type_handle;
IMPLEMENT_CLASS(LerpParticleFunction);
IMPLEMENT_CLASS(VelocityJitterParticleFunction);
IMPLEMENT_CLASS(BounceParticleFunction);

/**
 *
 */
MotionParticleFunction::
MotionParticleFunction(PN_stdfloat drag) :
  _drag(drag)
{
}

/**
 * Adds the current velocity of the particle to the particle's position.
 */
void MotionParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {

  if (!system->_forces.empty()) {
    // The particle system has forces.  We need to accumulate them and
    // integrate.

    LVector3 *force_accum = (LVector3 *)alloca(sizeof(LVector3) * system->_num_alive_particles);
    memset(force_accum, 0, sizeof(LVector3) * system->_num_alive_particles);

    // Accumulate forces.
    for (ParticleForce2 *force : system->_forces) {
      force->accumulate(1.0f, force_accum, system);
    }

    // Integrate forces.
    for (Particle &p : system->_particles) {
      if (!p._alive) {
        continue;
      }

      p._prev_pos = p._pos;
      p._pos += (p._velocity * dt) + (*force_accum * dt * dt * 0.5);
      p._velocity += *force_accum * dt;

      // Add rotation speed.
      p._rotation += p._rotation_speed * dt;

      ++force_accum;
    }

  } else {
    // No forces on particle system, simply add current velocity onto
    // particle position.
    for (Particle &p : system->_particles) {
      if (!p._alive) {
        continue;
      }

      p._prev_pos = p._pos;
      p._pos += p._velocity * dt;
      p._rotation += p._rotation_speed * dt;
    }
  }
}

/**
 *
 */
void LifespanKillerParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {
  for (int i = 0; i < (int)system->_particles.size(); ++i) {
    const Particle *p = &system->_particles[i];
    if (!p->_alive) {
      continue;
    }

    double elapsed = time - (double)p->_spawn_time;
    if (elapsed >= p->_duration) {
      // It's time for this particle to die.
      system->kill_particle(i);
    }
  }
}

/**
 *
 */
LerpParticleFunction::
LerpParticleFunction(Component component) :
  _component(component)
{
}

/**
 *
 */
void LerpParticleFunction::
add_constant_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &value) {
  LerpSegment seg;
  seg._start_frac = start;
  seg._end_frac = end;
  seg._type = LT_constant;
  seg._start_value = value;
  _segments.push_back(seg);
}

/**
 *
 */
void LerpParticleFunction::
add_linear_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                   const LVecBase3 &end_value) {
  LerpSegment seg;
  seg._type = LT_linear;
  seg._start_frac = start;
  seg._end_frac = end;
  seg._start_value = start_value;
  seg._end_value = end_value;
  _segments.push_back(seg);
}

/**
 *
 */
void LerpParticleFunction::
add_exponential_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                        const LVecBase3 &end_value, PN_stdfloat exponent) {
  LerpSegment seg;
  seg._type = LT_exponential;
  seg._start_frac = start;
  seg._end_frac = end;
  seg._start_value = start_value;
  seg._end_value = end_value;
  seg._func_data[0] = exponent;
  _segments.push_back(seg);
}

/**
 *
 */
void LerpParticleFunction::
add_stepwave_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                     const LVecBase3 &end_value, PN_stdfloat start_width, PN_stdfloat end_width) {
  LerpSegment seg;
  seg._type = LT_stepwave;
  seg._start_frac = start;
  seg._end_frac = end;
  seg._start_value = start_value;
  seg._end_value = end_value;
  seg._func_data[0] = start_width;
  seg._func_data[1] = end_width;
  _segments.push_back(seg);
}

/**
 *
 */
void LerpParticleFunction::
add_sinusoid_segment(PN_stdfloat start, PN_stdfloat end, const LVecBase3 &start_value,
                     const LVecBase3 &end_value, PN_stdfloat period) {
  LerpSegment seg;
  seg._type = LT_sinusoid;
  seg._start_frac = start;
  seg._end_frac = end;
  seg._start_value = start_value;
  seg._end_value = end_value;
  seg._func_data[0] = period;
  _segments.push_back(seg);
}

/**
 *
 */
void LerpParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {
  LVecBase3 value;

  for (int i = 0; i < (int)system->_particles.size(); ++i) {
    Particle *p = &system->_particles[i];
    if (!p->_alive) {
      continue;
    }

    PN_stdfloat elapsed = (PN_stdfloat)time - p->_spawn_time;
    PN_stdfloat frac = elapsed / p->_duration;

    for (const LerpSegment &seg : _segments) {
      if (frac < seg._start_frac || frac > seg._end_frac) {
        continue;
      }

      // Remap 0-1 particle lifespan fraction to 0-1 segment fraction.
      PN_stdfloat remapped_frac = remap_val_clamped(
        frac, seg._start_frac, seg._end_frac, 0.0f, 1.0f);

      // Evaluate lerp function.
      switch (seg._type) {
      case LT_constant:
        value = seg._start_value;
        break;

      case LT_linear:
        value = seg._start_value * (1.0f - remapped_frac) + seg._end_value * remapped_frac;
        break;

      case LT_exponential:
        {
          PN_stdfloat exp_frac = std::pow(remapped_frac, seg._func_data[0]);
          value = seg._start_value * (1.0f - exp_frac) + seg._end_value * exp_frac;
        }
        break;

      case LT_stepwave:
        if (fmodf(remapped_frac, seg._func_data[0] + seg._func_data[1]) > seg._func_data[0]) {
          value = seg._start_value;
        } else {
          value = seg._end_value;
        }
        break;

      case LT_sinusoid:
        {
          PN_stdfloat weight_a = (1.0f + std::cos(remapped_frac * MathNumbers::pi_f * 2.0f / seg._func_data[0])) * 0.5f;
          value = seg._start_value * weight_a + seg._end_value * (1.0f - weight_a);
        }
        break;
      }

      // Store lerped value on specified particle component.
      switch (_component) {
      case C_rgb:
        p->_color = LColor(value, p->_color[3]);
        break;
      case C_alpha:
        p->_color[3] = value[0];
        break;
      case C_scale:
        p->_scale = value.get_xy();
        break;
      case C_rotation:
        p->_rotation = value[0];
        break;
      }
    }
  }
}

/**
 *
 */
VelocityJitterParticleFunction::
VelocityJitterParticleFunction(PN_stdfloat amp_min, PN_stdfloat amp_max) :
  _amplitude_min(amp_min),
  _amplitude_range(amp_max - amp_min)
{
}

/**
 *
 */
void VelocityJitterParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    // Instantaneous random velocity modification.
    p._velocity += p2_random_unit_vector() * p2_random_min_range(_amplitude_min, _amplitude_range);
  }
}

/**
 *
 */
BounceParticleFunction::
BounceParticleFunction(const LPlane &plane, PN_stdfloat bounciness) :
  _plane(plane),
  _bounciness(bounciness)
{
}

/**
 *
 */
void BounceParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {
  LVector3 normal = _plane.get_normal();

  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    LVector3 particle_dir = p._velocity.normalized();

    if (IS_NEARLY_ZERO(particle_dir.length_squared())) {
      continue;
    }

    PN_stdfloat dist = _plane.dist_to_plane(p._pos + p._velocity * dt);
    if (dist <= 0.0f) {
      // Hit plane, bounce.
      LVector3 reflect = 2.0f * normal * normal.dot(p._velocity) - p._velocity;
      p._velocity = -reflect * _bounciness;
    }
  }
}
