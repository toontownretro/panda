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
ParticleForce2::
ParticleForce2() :
  _axis_mask(AM_all)
{
}

/**
 * Sets the mask of vector axes that the force should apply to.
 * This can be used to limit a force to only the X axis, for example.
 */
void ParticleForce2::
set_axis_mask(unsigned int mask) {
  _axis_mask = mask;
}

/**
 *
 */
void ParticleForce2::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_uint8(_axis_mask);
}

/**
 *
 */
void ParticleForce2::
fillin(DatagramIterator &scan, BamReader *manager) {
  _axis_mask = scan.get_uint8();
}

/**
 *
 */
VectorParticleForce::
VectorParticleForce(const LVector3 &force, PN_stdfloat start, PN_stdfloat end) :
  _force(force),
  _start(start),
  _end(end)
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

    PN_stdfloat elapsed = system->_elapsed - p._spawn_time;
    PN_stdfloat frac = elapsed / p._duration;
    if (frac < _start || frac > _end) {
      ++accum;
      continue;
    }

    *accum += apply_axis_mask(_force * strength);
    ++accum;
  }
}

/**
 *
 */
void VectorParticleForce::
write_datagram(BamWriter *manager, Datagram &me) {
  ParticleForce2::write_datagram(manager, me);
  _force.write_datagram(me);
  me.add_stdfloat(_start);
  me.add_stdfloat(_end);
}

/**
 *
 */
void VectorParticleForce::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParticleForce2::fillin(scan, manager);
  _force.read_datagram(scan);
  _start = scan.get_stdfloat();
  _end = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *VectorParticleForce::
make_from_bam(const FactoryParams &params) {
  VectorParticleForce *obj = new VectorParticleForce(0.0f);

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void VectorParticleForce::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
CylinderVortexParticleForce::
CylinderVortexParticleForce(PN_stdfloat coef, const LVector3 &axis, const LPoint3 &center) :
  _coef(coef),
  _axis(axis),
  _center(center),
  _input0(-1),
  _input1(-1),
  _mode(AM_explicit),
  _local_axis(false)
{
}

/**
 *
 */
void CylinderVortexParticleForce::
set_local_axis(bool flag) {
  _local_axis = flag;
}

/**
 *
 */
void CylinderVortexParticleForce::
set_input0(int input) {
  _input0 = input;
}

/**
 *
 */
void CylinderVortexParticleForce::
set_input1(int input) {
  _input1 = input;
}

/**
 *
 */
void CylinderVortexParticleForce::
set_mode(AxisMode mode) {
  _mode = mode;
}

/**
 *
 */
void CylinderVortexParticleForce::
accumulate(PN_stdfloat strength, LVector3 *accum, ParticleSystem2 *system) {
  LVector3 world_axis;

  switch (_mode) {
  case AM_explicit:
    world_axis = _axis;
    if (_local_axis && _input0 != -1) {
      // Twist axis specified relative to an input node.
      world_axis = system->get_input_value(_input0)->get_mat().xform_vec(world_axis);
    }
    break;
  case AM_input:
    // Forward vector of input.
    world_axis = system->get_input_value(_input0)->get_quat().get_forward();
    break;
  case AM_vec_between_inputs:
    // Vector between two inputs.
    world_axis = system->get_input_value(_input1)->get_pos() - system->get_input_value(_input0)->get_pos();
    break;
  }

  world_axis.normalize();

  LPoint3 center = _center;
  if (_input0 != -1) {
    // Center point relative to an input node.
    center = system->get_input_value(_input0)->get_mat().xform_point(center);
  }

  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    LVector3 offset = p._pos - center;
    if (!offset.normalize()) {
      ++accum;
      continue;
    }
    LVector3 parallel = offset;
    LVector3 axis_offset = offset;
    axis_offset.componentwise_mult(world_axis);
    parallel.componentwise_mult(axis_offset);
    offset -= parallel;
    if (!offset.normalize()) {
      ++accum;
      continue;
    }

    LVector3 tangential = offset.cross(world_axis);
    tangential *= strength * _coef;

    *accum += apply_axis_mask(tangential);
    ++accum;
  }
}

/**
 *
 */
void CylinderVortexParticleForce::
write_datagram(BamWriter *manager, Datagram &me) {
  ParticleForce2::write_datagram(manager, me);
  me.add_uint8(_mode);
  me.add_int8(_input0);
  me.add_int8(_input1);
  _axis.write_datagram(me);
  me.add_bool(_local_axis);
  me.add_stdfloat(_coef);
  _center.write_datagram(me);
}

/**
 *
 */
void CylinderVortexParticleForce::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParticleForce2::fillin(scan, manager);
  _mode = (AxisMode)scan.get_uint8();
  _input0 = scan.get_int8();
  _input1 = scan.get_int8();
  _axis.read_datagram(scan);
  _local_axis = scan.get_bool();
  _coef = scan.get_stdfloat();
  _center.read_datagram(scan);
}

/**
 *
 */
TypedWritable *CylinderVortexParticleForce::
make_from_bam(const FactoryParams &params) {
  CylinderVortexParticleForce *obj = new CylinderVortexParticleForce;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void CylinderVortexParticleForce::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
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

    *accum += apply_axis_mask(p2_random_unit_vector()) * strength * _amplitude;
    ++accum;
  }
}

/**
 *
 */
void JitterParticleForce::
write_datagram(BamWriter *manager, Datagram &me) {
  ParticleForce2::write_datagram(manager, me);
  me.add_stdfloat(_amplitude);
  me.add_stdfloat(_start);
  me.add_stdfloat(_end);
}

/**
 *
 */
void JitterParticleForce::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParticleForce2::fillin(scan, manager);
  _amplitude = scan.get_stdfloat();
  _start = scan.get_stdfloat();
  _end = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *JitterParticleForce::
make_from_bam(const FactoryParams &params) {
  JitterParticleForce *obj = new JitterParticleForce;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void JitterParticleForce::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
AttractParticleForce::
AttractParticleForce(int input, const LPoint3 &point, PN_stdfloat falloff, PN_stdfloat amplitude,
                     PN_stdfloat radius) :
  _input(input),
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
  LPoint3 ps_space_point;
  if (_input != -1) {
    // Get system-space point relative to an input node.  Allows the particles
    // to attract to a moving target in the game world, for instance.
    ps_space_point = system->get_input_value(_input)->get_mat().xform_point(_point);
  } else {
    // Static point, already in system-space.
    ps_space_point = _point;
  }

  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    // Attract to force point.
    LVector3 vec = p._pos - ps_space_point;
    PN_stdfloat len = vec.length();
    if (IS_NEARLY_ZERO(len)) {
      accum++;
      continue;
    }
    if (_radius <= 0.0f) {
      vec /= len;
    }
    vec *= -_amplitude * strength;
    if (_radius > 0.0f) {
      vec /= std::pow(_radius, _falloff);
    } else {
      vec /= std::pow(len, _falloff);
    }
    *accum += apply_axis_mask(vec);
    ++accum;
  }
}

/**
 *
 */
void AttractParticleForce::
write_datagram(BamWriter *manager, Datagram &me) {
  ParticleForce2::write_datagram(manager, me);
  me.add_int8(_input);
  _point.write_datagram(me);
  me.add_stdfloat(_amplitude);
  me.add_stdfloat(_falloff);
  me.add_stdfloat(_radius);
}

/**
 *
 */
void AttractParticleForce::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParticleForce2::fillin(scan, manager);
  _input = scan.get_int8();
  _point.read_datagram(scan);
  _amplitude = scan.get_stdfloat();
  _falloff = scan.get_stdfloat();
  _radius = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *AttractParticleForce::
make_from_bam(const FactoryParams &params) {
  AttractParticleForce *obj = new AttractParticleForce;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void AttractParticleForce::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
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

    *accum -= apply_axis_mask(p._velocity) * _coef;
    ++accum;
  }
}

/**
 *
 */
void FrictionParticleForce::
write_datagram(BamWriter *manager, Datagram &me) {
  ParticleForce2::write_datagram(manager, me);
  me.add_stdfloat(_coef);
}

/**
 *
 */
void FrictionParticleForce::
fillin(DatagramIterator &scan, BamReader *manager) {
  ParticleForce2::fillin(scan, manager);
  _coef = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *FrictionParticleForce::
make_from_bam(const FactoryParams &params) {
  FrictionParticleForce *obj = new FrictionParticleForce(0.0f);

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void FrictionParticleForce::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}
