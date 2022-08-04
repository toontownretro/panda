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
IMPLEMENT_CLASS(LinearMotionParticleFunction);
IMPLEMENT_CLASS(AngularMotionParticleFunction);
TypeHandle LifespanKillerParticleFunction::_type_handle;
IMPLEMENT_CLASS(LerpParticleFunction);
IMPLEMENT_CLASS(VelocityJitterParticleFunction);
IMPLEMENT_CLASS(BounceParticleFunction);

/**
 *
 */
LinearMotionParticleFunction::
LinearMotionParticleFunction(PN_stdfloat drag) :
  _drag(drag)
{
}

/**
 * Adds the current velocity of the particle to the particle's position.
 */
void LinearMotionParticleFunction::
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

      LVector3 accel_vec = *force_accum;

      p._prev_pos = p._pos;
      p._pos += (p._velocity * dt) + (accel_vec * dt * dt * 0.5);
      p._velocity += accel_vec * dt;

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
    }
  }

  // Enforce constraints.
  if (!system->_constraints.empty() && system->_num_alive_particles > 0) {
    bool constraint_satisifed[100];
    bool final_constraint[100];
    for (size_t i = 0; i < system->_constraints.size() && i < 100u; ++i) {
      // TODO: final constraints.

      constraint_satisifed[i] = false;
    }

    for (int p = 0; p < 3; ++p) {
      for (size_t i = 0; i < system->_constraints.size() && i < 100u; ++i) {
        if (!constraint_satisifed[i]) {
          ParticleConstraint2 *constraint = system->_constraints[i];
          bool did_something = constraint->enforce_constraint(time, dt, system);
          if (did_something) {
            // Invalidate other constraints.
            for (size_t j = 0; j < system->_constraints.size() && j < 100; ++j) {
              if (i != j) {
                constraint_satisifed[j] = false;
              }
            }
          }
        }
      }
    }

    // TODO: run final constraints
  }
}

/**
 *
 */
TypedWritable *LinearMotionParticleFunction::
make_from_bam(const FactoryParams &params) {
  LinearMotionParticleFunction *obj = new LinearMotionParticleFunction;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void LinearMotionParticleFunction::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void AngularMotionParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {
  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    p._rotation += p._rotation_speed * dt;
  }
}

/**
 *
 */
TypedWritable *AngularMotionParticleFunction::
make_from_bam(const FactoryParams &params) {
  AngularMotionParticleFunction *obj = new AngularMotionParticleFunction;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void AngularMotionParticleFunction::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
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
TypedWritable *LifespanKillerParticleFunction::
make_from_bam(const FactoryParams &params) {
  LifespanKillerParticleFunction *obj = new LifespanKillerParticleFunction;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void LifespanKillerParticleFunction::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
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
add_segment(const ParticleLerpSegment &seg) {
  _segments.push_back(seg);
}

/**
 *
 */
void LerpParticleFunction::
update(double time, double dt, ParticleSystem2 *system) {
  LVecBase3 value;
  LVecBase3 start_value, end_value;

  for (int i = 0; i < (int)system->_particles.size(); ++i) {
    Particle *p = &system->_particles[i];
    if (!p->_alive) {
      continue;
    }

    PN_stdfloat elapsed = (PN_stdfloat)time - p->_spawn_time;
    PN_stdfloat frac = elapsed / p->_duration;

    for (const ParticleLerpSegment &seg : _segments) {
      if (frac < seg._start_frac || frac > seg._end_frac) {
        continue;
      }

      // Remap 0-1 particle lifespan fraction to 0-1 segment fraction.
      PN_stdfloat remapped_frac = remap_val_clamped(
        frac, seg._start_frac, seg._end_frac, 0.0f, 1.0f);

      if (!seg._start_is_initial) {
        start_value = seg._start_value;

      } else {
        switch (_component) {
        case C_rgb:
          start_value = p->_initial_color.get_xyz();
          break;
        case C_alpha:
          start_value = LVecBase3(p->_initial_color[3]);
          break;
        case C_scale:
          start_value = LVecBase3(p->_initial_scale, 1.0f);
          break;
        case C_rotation:
          start_value = LVecBase3(p->_initial_rotation);
          break;
        }
      }

      if (!seg._end_is_initial) {
        end_value = seg._end_value;

      } else {
        switch (_component) {
        case C_rgb:
          end_value = p->_initial_color.get_xyz();
          break;
        case C_alpha:
          end_value = LVecBase3(p->_initial_color[3]);
          break;
        case C_scale:
          end_value = LVecBase3(p->_initial_scale, 1.0f);
          break;
        case C_rotation:
          end_value = LVecBase3(p->_initial_rotation);
          break;
        }
      }

      // Evaluate lerp function.
      switch (seg._type) {
      case ParticleLerpSegment::LT_constant:
        value = start_value;
        break;

      case ParticleLerpSegment::LT_linear:
        value = start_value * (1.0f - remapped_frac) + end_value * remapped_frac;
        break;

      case ParticleLerpSegment::LT_exponential:
        {
          PN_stdfloat exp_frac = std::pow(remapped_frac, seg._func_data[0]);
          value = start_value * (1.0f - exp_frac) + end_value * exp_frac;
        }
        break;

      case ParticleLerpSegment::LT_stepwave:
        if (fmodf(remapped_frac, seg._func_data[0] + seg._func_data[1]) > seg._func_data[0]) {
          value = start_value;
        } else {
          value = end_value;
        }
        break;

      case ParticleLerpSegment::LT_sinusoid:
        {
          PN_stdfloat weight_a = (1.0f + std::cos(remapped_frac * MathNumbers::pi_f * 2.0f / seg._func_data[0])) * 0.5f;
          value = start_value * weight_a + end_value * (1.0f - weight_a);
        }
        break;
      }

      if (seg._scale_on_initial) {
        switch (_component) {
        case C_rgb:
          value[0] *= p->_initial_color[0];
          value[1] *= p->_initial_color[1];
          value[2] *= p->_initial_color[2];
          break;
        case C_alpha:
          value[0] *= p->_initial_color[3];
          break;
        case C_scale:
          value[0] *= p->_initial_scale[0];
          value[1] *= p->_initial_scale[1];
          break;
        case C_rotation:
          value[0] *= p->_initial_rotation;
          break;
        }
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
void LerpParticleFunction::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_uint8(_component);
  me.add_uint8(_segments.size());
  for (const ParticleLerpSegment &seg : _segments) {
    me.add_stdfloat(seg._start_frac);
    me.add_stdfloat(seg._end_frac);
    me.add_uint8(seg._type);
    me.add_bool(seg._start_is_initial);
    me.add_bool(seg._end_is_initial);
    if (!seg._start_is_initial) {
      seg._start_value.write_datagram(me);
    }
    if (!seg._end_is_initial) {
      seg._end_value.write_datagram(me);
    }
    me.add_bool(seg._scale_on_initial);
    me.add_stdfloat(seg._func_data[0]);
    me.add_stdfloat(seg._func_data[1]);
  }
}

/**
 *
 */
void LerpParticleFunction::
fillin(DatagramIterator &scan, BamReader *manager) {
  _component = (Component)scan.get_uint8();
  _segments.resize(scan.get_uint8());
  for (size_t i = 0; i < _segments.size(); ++i) {
    ParticleLerpSegment &seg = _segments[i];
    seg._start_frac = scan.get_stdfloat();
    seg._end_frac = scan.get_stdfloat();
    seg._type = (ParticleLerpSegment::LerpType)scan.get_uint8();
    seg._start_is_initial = scan.get_bool();
    seg._end_is_initial = scan.get_bool();
    if (!seg._start_is_initial) {
      seg._start_value.read_datagram(scan);
    }
    if (!seg._end_is_initial) {
      seg._end_value.read_datagram(scan);
    }
    seg._scale_on_initial = scan.get_bool();
    seg._func_data[0] = scan.get_stdfloat();
    seg._func_data[1] = scan.get_stdfloat();
  }
}

/**
 *
 */
TypedWritable *LerpParticleFunction::
make_from_bam(const FactoryParams &params) {
  LerpParticleFunction *obj = new LerpParticleFunction;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void LerpParticleFunction::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
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
void VelocityJitterParticleFunction::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_stdfloat(_amplitude_min);
  me.add_stdfloat(_amplitude_range);
}

/**
 *
 */
void VelocityJitterParticleFunction::
fillin(DatagramIterator &scan, BamReader *manager) {
  _amplitude_min = scan.get_stdfloat();
  _amplitude_range = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *VelocityJitterParticleFunction::
make_from_bam(const FactoryParams &params) {
  VelocityJitterParticleFunction *obj = new VelocityJitterParticleFunction;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void VelocityJitterParticleFunction::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
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

/**
 *
 */
void BounceParticleFunction::
write_datagram(BamWriter *manager, Datagram &me) {
  _plane.write_datagram(me);
  me.add_stdfloat(_bounciness);
}

/**
 *
 */
void BounceParticleFunction::
fillin(DatagramIterator &scan, BamReader *manager) {
  _plane.read_datagram(scan);
  _bounciness = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *BounceParticleFunction::
make_from_bam(const FactoryParams &params) {
  BounceParticleFunction *obj = new BounceParticleFunction;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void BounceParticleFunction::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}
