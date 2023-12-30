/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleInitializer2.cxx
 * @author brian
 * @date 2022-04-04
 */

#include "particleInitializer2.h"
#include "particleSystem2.h"
#include "p2_utils.h"
#include "nodePath.h"

#define BAM_WRITE(clsname) \
void clsname:: \
write_datagram(BamWriter *manager, Datagram &me)

#define BAM_READ(clsname) \
void clsname:: \
fillin(DatagramIterator &scan, BamReader *manager)

#define BAM_READ_FACTORY(clsname) \
void clsname:: \
register_with_read_factory() { \
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam); \
} \
TypedWritable *clsname:: \
make_from_bam(const FactoryParams &params) { \
  clsname *obj = new clsname; \
  DatagramIterator scan; \
  BamReader *manager; \
  parse_params(params, scan, manager); \
  obj->fillin(scan, manager); \
  return obj; \
}

IMPLEMENT_CLASS(ParticleInitializer2);

IMPLEMENT_CLASS(P2_INIT_LifespanRandomRange);

IMPLEMENT_CLASS(P2_INIT_PositionExplicit); // PointEmitter
IMPLEMENT_CLASS(P2_INIT_PositionBoxVolume); // BoxEmitter+RectangleEmitter
IMPLEMENT_CLASS(P2_INIT_PositionSphereVolume); // SphereVolumeEmitter+SphereSurfaceEmitter+DiscEmitter+RingEmitter+ArcEmitter
IMPLEMENT_CLASS(P2_INIT_PositionLineSegment); // LineEmitter
IMPLEMENT_CLASS(P2_INIT_PositionParametricCurve);
IMPLEMENT_CLASS(P2_INIT_PositionCharacterJoints);

IMPLEMENT_CLASS(P2_INIT_VelocityExplicit); // ET_EXPLICIT
IMPLEMENT_CLASS(P2_INIT_VelocityCone); //
IMPLEMENT_CLASS(P2_INIT_VelocityRadiate); // ET_RADIATE

IMPLEMENT_CLASS(P2_INIT_RotationRandomRange);
IMPLEMENT_CLASS(P2_INIT_RotationVelocityRandomRange);

IMPLEMENT_CLASS(P2_INIT_ScaleRandomRange);
IMPLEMENT_CLASS(P2_INIT_ColorRandomRange);
IMPLEMENT_CLASS(P2_INIT_AlphaRandomRange);

IMPLEMENT_CLASS(P2_INIT_RemapAttribute);

IMPLEMENT_CLASS(P2_INIT_PositionModelHitBoxes);

IMPLEMENT_CLASS(P2_INIT_AnimationIndexRandom);

/**
 *
 */
P2_INIT_LifespanRandomRange::
P2_INIT_LifespanRandomRange(PN_stdfloat ls_min, PN_stdfloat ls_max, PN_stdfloat exponent) :
  _lifespan_min(ls_min),
  _lifespan_range(ls_max - ls_min),
  _lifespan_exponent(exponent)
{
}

/**
 *
 */
void P2_INIT_LifespanRandomRange::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    if (_lifespan_exponent != 1.0f) {
      p->_duration = p2_random_min_range(_lifespan_min, _lifespan_range, _lifespan_exponent);
    } else {
      p->_duration = p2_random_min_range(_lifespan_min, _lifespan_range);
    }
  }
}

BAM_WRITE(P2_INIT_LifespanRandomRange) {
  me.add_stdfloat(_lifespan_min);
  me.add_stdfloat(_lifespan_range);
  me.add_stdfloat(_lifespan_exponent);
}
BAM_READ(P2_INIT_LifespanRandomRange) {
  _lifespan_min = scan.get_stdfloat();
  _lifespan_range = scan.get_stdfloat();
  _lifespan_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_LifespanRandomRange);

/**
 *
 */
P2_INIT_PositionExplicit::
P2_INIT_PositionExplicit(const LPoint3 &point) :
  _point(point)
{
}

/**
 *
 */
void P2_INIT_PositionExplicit::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  LPoint3 origin_ps_space = emission_xform.xform_point(_point);

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_pos = origin_ps_space;
  }
}

BAM_WRITE(P2_INIT_PositionExplicit) {
  _point.write_datagram(me);
}
BAM_READ(P2_INIT_PositionExplicit) {
  _point.read_datagram(scan);
}
BAM_READ_FACTORY(P2_INIT_PositionExplicit);

/**
 *
 */
P2_INIT_PositionBoxVolume::
P2_INIT_PositionBoxVolume(const LPoint3 &mins, const LPoint3 &maxs) :
  _mins(mins),
  _maxs(maxs)
{
}

/**
 *
 */
void P2_INIT_PositionBoxVolume::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    p->_pos[0] = p2_random_min_max(_mins[0], _maxs[0]);
    p->_pos[1] = p2_random_min_max(_mins[1], _maxs[1]);
    p->_pos[2] = p2_random_min_max(_mins[2], _maxs[2]);
    emission_xform.xform_point_in_place(p->_pos);
  }
}

BAM_WRITE(P2_INIT_PositionBoxVolume) {
  _mins.write_datagram(me);
  _maxs.write_datagram(me);
}
BAM_READ(P2_INIT_PositionBoxVolume) {
  _mins.read_datagram(scan);
  _maxs.read_datagram(scan);
}
BAM_READ_FACTORY(P2_INIT_PositionBoxVolume);

/**
 *
 */
P2_INIT_PositionSphereVolume::
P2_INIT_PositionSphereVolume(const LPoint3 &center, PN_stdfloat radius_min, PN_stdfloat radius_max,
                             const LVecBase3 &bias, const LVecBase3 &scale,
                             const LVecBase3i &absolute_value) :
  _center(center),
  _radius_min(radius_min),
  _radius_range(radius_max - radius_min),
  _bias(bias),
  _scale(scale),
  _absolute_value(absolute_value)
{
}

/**
 *
 */
void P2_INIT_PositionSphereVolume::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {

  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    // Pick random direction vector.
    LVector3 vec = p2_random_unit_vector();

    // Take absolute values of requested axes, to create hemisphere/
    // quarter-sphere volumes.
    if (_absolute_value[0]) {
      vec[0] = std::abs(vec[0]);
    }
    if (_absolute_value[1]) {
      vec[1] = std::abs(vec[1]);
    }
    if (_absolute_value[2]) {
      vec[2] = std::abs(vec[2]);
    }

    // Bias towards a particular direction to create rings/arcs.
    vec.componentwise_mult(_bias);
    vec.normalize();

    vec *= p2_random_min_range(_radius_min, _radius_range);

    // Scale the offset vector to create ovals, arches, etc.
    vec.componentwise_mult(_scale);

    p->_pos = emission_xform.xform_point(_center + vec);
  }
}

BAM_WRITE(P2_INIT_PositionSphereVolume) {
  _center.write_datagram(me);
  me.add_stdfloat(_radius_min);
  me.add_stdfloat(_radius_range);
  _bias.write_datagram(me);
  _scale.write_datagram(me);
  _absolute_value.write_datagram(me);
}
BAM_READ(P2_INIT_PositionSphereVolume) {
  _center.read_datagram(scan);
  _radius_min = scan.get_stdfloat();
  _radius_range = scan.get_stdfloat();
  _bias.read_datagram(scan);
  _scale.read_datagram(scan);
  _absolute_value.read_datagram(scan);
}
BAM_READ_FACTORY(P2_INIT_PositionSphereVolume);

/**
 *
 */
P2_INIT_PositionLineSegment::
P2_INIT_PositionLineSegment(const LPoint3 &a, const LPoint3 &b) :
  _a(a),
  _b(b)
{
}

/**
 *
 */
void P2_INIT_PositionLineSegment::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    // Pick random point along line segment.
    PN_stdfloat frac = p2_normalized_rand();
    p->_pos = emission_xform.xform_point(_a * (1.0f - frac) + _b * frac);
  }
}

BAM_WRITE(P2_INIT_PositionLineSegment) {
  _a.write_datagram(me);
  _b.write_datagram(me);
}
BAM_READ(P2_INIT_PositionLineSegment) {
  _a.read_datagram(scan);
  _b.read_datagram(scan);
}
BAM_READ_FACTORY(P2_INIT_PositionLineSegment);

/**
 *
 */
P2_INIT_PositionParametricCurve::
P2_INIT_PositionParametricCurve(ParametricCurve *curve) :
  _curve(curve)
{
}

/**
 *
 */
void P2_INIT_PositionParametricCurve::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  nassertv(_curve != nullptr);

  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  PN_stdfloat max_t = _curve->get_max_t();
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    // Random parametric point along curve.
    PN_stdfloat t = p2_normalized_rand() * max_t;
    // Evaluate position on curve at parametric point.
    _curve->get_point(t, p->_pos);
    emission_xform.xform_point_in_place(p->_pos);
  }
}

BAM_WRITE(P2_INIT_PositionParametricCurve) {
  manager->write_pointer(me, _curve);
}
BAM_READ(P2_INIT_PositionParametricCurve) {
  manager->read_pointer(scan);
}
BAM_READ_FACTORY(P2_INIT_PositionParametricCurve);

/**
 *
 */
int P2_INIT_PositionParametricCurve::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;
  _curve = DCAST(ParametricCurve, p_list[pi++]);
  return pi;
}

/**
 *
 */
P2_INIT_PositionCharacterJoints::
P2_INIT_PositionCharacterJoints(Character *character, PN_stdfloat radius) :
  _char(character),
  _radius(radius)
{
}

/**
 *
 */
void P2_INIT_PositionCharacterJoints::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  NodePath char_np((PandaNode *)_char->get_node(0));
  LMatrix4 char_net_mat = char_np.get_mat(NodePath());

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    int joint = (int)(p2_normalized_rand() * (_char->get_num_joints() - 1));
    LMatrix4 joint_transform = _char->get_joint_net_transform(joint);
    joint_transform = char_net_mat * joint_transform;

    LVector3 offset = p2_random_unit_vector() * (p2_normalized_rand() * _radius);

    p->_pos = joint_transform.get_row3(3) + offset;
  }
}

/**
 *
 */
P2_INIT_VelocityExplicit::
P2_INIT_VelocityExplicit(const LVector3 &dir, PN_stdfloat amp_min, PN_stdfloat amp_max,
                         PN_stdfloat amp_exponent) :
  _vel(dir),
  _amplitude_min(amp_min),
  _amplitude_range(amp_max - amp_min),
  _amplitude_exponent(amp_exponent)
{
}

/**
 *
 */
void P2_INIT_VelocityExplicit::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_velocity += emission_xform.xform_vec(_vel * p2_random_min_range(_amplitude_min, _amplitude_range, _amplitude_exponent));
  }
}

BAM_WRITE(P2_INIT_VelocityExplicit) {
  _vel.write_datagram(me);
  me.add_stdfloat(_amplitude_min);
  me.add_stdfloat(_amplitude_range);
  me.add_stdfloat(_amplitude_exponent);
}
BAM_READ(P2_INIT_VelocityExplicit) {
  _vel.read_datagram(scan);
  _amplitude_min = scan.get_stdfloat();
  _amplitude_range = scan.get_stdfloat();
  _amplitude_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_VelocityExplicit);

/**
 *
 */
P2_INIT_VelocityCone::
P2_INIT_VelocityCone(const LVecBase3 &min_hpr, const LVecBase3 &max_hpr,
                     PN_stdfloat min_amplitude, PN_stdfloat max_amplitude,
                     PN_stdfloat amplitude_exponent) :
  _min_hpr(min_hpr),
  _max_hpr(max_hpr),
  _min_amplitude(min_amplitude),
  _amplitude_range(max_amplitude - min_amplitude),
  _amplitude_exponent(amplitude_exponent)
{
}

/**
 *
 */
void P2_INIT_VelocityCone::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    // Pick random pitch and yaw within HPR cone.
    PN_stdfloat yaw = p2_random_min_max(_min_hpr[0], _max_hpr[0]);
    PN_stdfloat pitch = p2_random_min_max(_min_hpr[1], _max_hpr[1]);

    // Put in quat to get velocity direction.
    LQuaternion q;
    q.set_hpr(LVecBase3(yaw, pitch, 0.0f));

    // Pick random amplitude within given range.
    PN_stdfloat amplitude = p2_random_min_range(_min_amplitude, _amplitude_range, _amplitude_exponent);

    // Construct velocity vector.
    p->_velocity += emission_xform.xform_vec(q.get_forward() * amplitude);
  }
}

BAM_WRITE(P2_INIT_VelocityCone) {
  _min_hpr.write_datagram(me);
  _max_hpr.write_datagram(me);
  me.add_stdfloat(_min_amplitude);
  me.add_stdfloat(_amplitude_range);
  me.add_stdfloat(_amplitude_exponent);
}
BAM_READ(P2_INIT_VelocityCone) {
  _min_hpr.read_datagram(scan);
  _max_hpr.read_datagram(scan);
  _min_amplitude = scan.get_stdfloat();
  _amplitude_range = scan.get_stdfloat();
  _amplitude_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_VelocityCone);

/**
 *
 */
P2_INIT_VelocityRadiate::
P2_INIT_VelocityRadiate(const LPoint3 &point, PN_stdfloat min_amp, PN_stdfloat max_amp,
                        PN_stdfloat amp_exp) :
  _point(point),
  _min_amplitude(min_amp),
  _amplitude_range(max_amp - min_amp),
  _amplitude_exponent(amp_exp)
{
}

/**
 *
 */
void P2_INIT_VelocityRadiate::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  // Matrix to transform from emission space to particle system space.
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();

  // Transform radiate origin from emission space into particle system space,
  // since we're constructing the velocity vector from the particle's position,
  // which is already in particle system space.
  LPoint3 origin_ps_space = emission_xform.xform_point(_point);

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    LVector3 vec = p->_pos - origin_ps_space;
    if (!vec.normalize()) {
      // Arbitrary direction.
      vec = LVector3::up();
    }

    PN_stdfloat amplitude = p2_random_min_range(_min_amplitude, _amplitude_range, _amplitude_exponent);

    p->_velocity += vec * amplitude;
  }
}

BAM_WRITE(P2_INIT_VelocityRadiate) {
  _point.write_datagram(me);
  me.add_stdfloat(_min_amplitude);
  me.add_stdfloat(_amplitude_range);
  me.add_stdfloat(_amplitude_exponent);
}
BAM_READ(P2_INIT_VelocityRadiate) {
  _point.read_datagram(scan);
  _min_amplitude = scan.get_stdfloat();
  _amplitude_range = scan.get_stdfloat();
  _amplitude_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_VelocityRadiate);

/**
 *
 */
P2_INIT_RotationRandomRange::
P2_INIT_RotationRandomRange(PN_stdfloat base, PN_stdfloat offset_min, PN_stdfloat offset_max,
                       PN_stdfloat offset_exponent) :
  _rot_base(base),
  _offset_min(offset_min),
  _offset_range(offset_max - offset_min),
  _offset_exponent(offset_exponent)
{
}

/**
 *
 */
void P2_INIT_RotationRandomRange::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    p->_rotation = _rot_base;
    if (_offset_exponent != 1.0f) {
      p->_rotation += p2_random_min_range(_offset_min, _offset_range, _offset_exponent);
    } else {
      p->_rotation += p2_random_min_range(_offset_min, _offset_range);
    }
  }
}

BAM_WRITE(P2_INIT_RotationRandomRange) {
  me.add_stdfloat(_rot_base);
  me.add_stdfloat(_offset_min);
  me.add_stdfloat(_offset_range);
  me.add_stdfloat(_offset_exponent);
}
BAM_READ(P2_INIT_RotationRandomRange) {
  _rot_base = scan.get_stdfloat();
  _offset_min = scan.get_stdfloat();
  _offset_range = scan.get_stdfloat();
  _offset_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_RotationRandomRange);

/**
 *
 */
P2_INIT_RotationVelocityRandomRange::
P2_INIT_RotationVelocityRandomRange(PN_stdfloat speed_min, PN_stdfloat speed_max,
                               PN_stdfloat speed_exponent, bool random_flip,
                               PN_stdfloat random_flip_chance,
                               PN_stdfloat random_flip_exponent) :
  _vel_min(speed_min),
  _vel_range(speed_max - speed_min),
  _vel_exponent(speed_exponent),
  _random_flip(random_flip),
  _random_flip_chance(random_flip_chance),
  _random_flip_exponent(random_flip_exponent)
{
}

/**
 *
 */
void P2_INIT_RotationVelocityRandomRange::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    PN_stdfloat speed;
    if (_vel_exponent != 1.0f) {
      speed = p2_random_min_range(_vel_min, _vel_range, _vel_exponent);
    } else {
      speed = p2_random_min_range(_vel_min, _vel_range);
    }

    if (_random_flip) {
      if (p2_normalized_rand(_random_flip_exponent) <= _random_flip_chance) {
        // Flip rotational velocity in opposite direction.
        speed = -speed;
      }
    }

    p->_rotation_speed += speed;
  }
}

BAM_WRITE(P2_INIT_RotationVelocityRandomRange) {
  me.add_stdfloat(_vel_min);
  me.add_stdfloat(_vel_range);
  me.add_stdfloat(_vel_exponent);
  me.add_bool(_random_flip);
  me.add_stdfloat(_random_flip_chance);
  me.add_stdfloat(_random_flip_exponent);
}
BAM_READ(P2_INIT_RotationVelocityRandomRange) {
  _vel_min = scan.get_stdfloat();
  _vel_range = scan.get_stdfloat();
  _vel_exponent = scan.get_stdfloat();
  _random_flip = scan.get_bool();
  _random_flip_chance = scan.get_stdfloat();
  _random_flip_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_RotationVelocityRandomRange);

/**
 *
 */
P2_INIT_ScaleRandomRange::
P2_INIT_ScaleRandomRange(const LVecBase3 &scale_min, const LVecBase3 &scale_max,
                    bool componentwise,
                    PN_stdfloat scale_exponent) :
  _scale_min(scale_min),
  _scale_range(scale_max - scale_min),
  _scale_exponent(scale_exponent),
  _componentwise(componentwise)
{
}

/**
 *
 */
void P2_INIT_ScaleRandomRange::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    if (_componentwise) {
      // Pick a random value for each scale component separately.
      p->_scale[0] = p2_random_min_range(_scale_min[0], _scale_range[0], _scale_exponent);
      p->_scale[1] = p2_random_min_range(_scale_min[1], _scale_range[1], _scale_exponent);
    } else {
      // Pick a single random value to lerp between the min and max scale uniformly.
      p->_scale = (_scale_min + _scale_range * p2_normalized_rand(_scale_exponent)).get_xz();
    }
  }
}

BAM_WRITE(P2_INIT_ScaleRandomRange) {
  _scale_min.write_datagram(me);
  _scale_range.write_datagram(me);
  me.add_stdfloat(_scale_exponent);
  me.add_bool(_componentwise);
}
BAM_READ(P2_INIT_ScaleRandomRange) {
  _scale_min.read_datagram(scan);
  _scale_range.read_datagram(scan);
  _scale_exponent = scan.get_stdfloat();
  _componentwise = scan.get_bool();
}
BAM_READ_FACTORY(P2_INIT_ScaleRandomRange);

/**
 *
 */
P2_INIT_ColorRandomRange::
P2_INIT_ColorRandomRange(const LVecBase3 &color_1, const LVecBase3 &color_2,
                         bool componentwise, PN_stdfloat exponent) :
  _color_min(color_1),
  _color_range(color_2 - color_1),
  _componentwise(componentwise),
  _exponent(exponent)
{
}

/**
 *
 */
void P2_INIT_ColorRandomRange::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    if (_componentwise) {
      // Pick a random value for each RGB component.
      p->_color[0] = p2_random_min_range(_color_min[0], _color_range[0], _exponent);
      p->_color[1] = p2_random_min_range(_color_min[1], _color_range[1], _exponent);
      p->_color[2] = p2_random_min_range(_color_min[2], _color_range[2], _exponent);

    } else {
      // Pick a single random fraction to lerp between the two colors.
      p->_color = LColor(_color_min + _color_range * p2_normalized_rand(_exponent), p->_color[3]);
    }
  }
}

BAM_WRITE(P2_INIT_ColorRandomRange) {
  _color_min.write_datagram(me);
  _color_range.write_datagram(me);
  me.add_stdfloat(_exponent);
  me.add_bool(_componentwise);
}
BAM_READ(P2_INIT_ColorRandomRange) {
  _color_min.read_datagram(scan);
  _color_range.read_datagram(scan);
  _exponent = scan.get_stdfloat();
  _componentwise = scan.get_bool();
}
BAM_READ_FACTORY(P2_INIT_ColorRandomRange);

/**
 *
 */
P2_INIT_AlphaRandomRange::
P2_INIT_AlphaRandomRange(PN_stdfloat alpha_min, PN_stdfloat alpha_max, PN_stdfloat exponent) :
  _alpha_min(alpha_min),
  _alpha_range(alpha_max - alpha_min),
  _alpha_exponent(exponent)
{
}

/**
 *
 */
void P2_INIT_AlphaRandomRange::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    p->_color[3] = p2_random_min_range(_alpha_min, _alpha_range, _alpha_exponent);
  }
}

BAM_WRITE(P2_INIT_AlphaRandomRange) {
  me.add_stdfloat(_alpha_min);
  me.add_stdfloat(_alpha_range);
  me.add_stdfloat(_alpha_exponent);
}
BAM_READ(P2_INIT_AlphaRandomRange) {
  _alpha_min = scan.get_stdfloat();
  _alpha_range = scan.get_stdfloat();
  _alpha_exponent = scan.get_stdfloat();
}
BAM_READ_FACTORY(P2_INIT_AlphaRandomRange);

/**
 *
 */
P2_INIT_RemapAttribute::
P2_INIT_RemapAttribute(Attribute src, int src_component, float src_min, float src_max,
                       Attribute dest, int dest_component, float dest_min, float dest_max) :
  _src_attrib(src),
  _src_component(src_component),
  _src_min(src_min),
  _src_range(src_max - src_min),
  _dest_attrib(dest),
  _dest_component(dest_component),
  _dest_min(dest_min),
  _dest_range(dest_max - dest_min),
  _mode(M_clamp),
  _spline(false)
{
}

/**
 *
 */
void P2_INIT_RemapAttribute::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  const LMatrix4 &emission_xform = system->get_input_value(0)->get_mat();
  const LMatrix4 *inv_emission = system->get_input_value(0)->get_inverse_mat();

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    float src;
    LPoint3 emission_pos;
    switch (_src_attrib) {
    case A_rgb:
      src = p->_color[_src_component];
      break;
    case A_alpha:
      src = p->_color[3];
      break;
    case A_pos:
      emission_pos = inv_emission->xform_point(p->_pos);
      src = emission_pos[_src_component];
      break;
    case A_scale:
      src = p->_pos[_src_component];
      break;
    case A_rotation:
      src = p->_rotation;
      break;
    case A_rotation_velocity:
      src = p->_rotation_speed;
      break;
    //case A_vel:
     // src = p->_velocity[_src_component];
     // break;
    }

    float cval = (src - _src_min) / _src_range;
    if (_mode == M_clamp) {
      cval = std::max(0.0f, std::min(1.0f, cval));
    } else if (_mode == M_ignore_out_of_range) {
      if (cval < 0.0f || cval > 1.0f) {
        continue;
      }
    }
    float dest = _dest_min;
    if (_spline) {
      dest += _dest_range * p2_simple_spline(cval);
    } else {
      dest += _dest_range * cval;
    }

    switch (_dest_attrib) {
    case A_rgb:
      p->_color[_dest_component] = dest;
      break;
    case A_alpha:
      p->_color[3] = dest;
      break;
    case A_pos:
      emission_pos = inv_emission->xform_point(p->_pos);
      emission_pos[_dest_component] = dest;
      p->_pos = emission_xform.xform_point(emission_pos);
      break;
    case A_scale:
      p->_scale[_dest_component] = dest;
      break;
    case A_rotation:
      p->_rotation = dest;
      break;
    case A_rotation_velocity:
      p->_rotation_speed = dest;
      break;
    //case A_vel:
    //  p->_velocity[_dest_component] = dest;
    //  break;
    }
  }
}

BAM_WRITE(P2_INIT_RemapAttribute) {
  me.add_uint8(_src_attrib);
  me.add_int8(_src_component);
  me.add_float32(_src_min);
  me.add_float32(_src_range);
  me.add_uint8(_dest_attrib);
  me.add_int8(_dest_component);
  me.add_float32(_dest_min);
  me.add_float32(_dest_range);
  me.add_uint8(_mode);
  me.add_bool(_spline);
}
BAM_READ(P2_INIT_RemapAttribute) {
  _src_attrib = (Attribute)scan.get_uint8();
  _src_component = scan.get_int8();
  _src_min = scan.get_float32();
  _src_range = scan.get_float32();
  _dest_attrib = (Attribute)scan.get_uint8();
  _dest_component = scan.get_int8();
  _dest_min = scan.get_float32();
  _dest_range = scan.get_float32();
  _mode = (Mode)scan.get_uint8();
  _spline = scan.get_bool();
}
BAM_READ_FACTORY(P2_INIT_RemapAttribute);

/**
 *
 */
P2_INIT_PositionModelHitBoxes::
P2_INIT_PositionModelHitBoxes(int model_root_input) :
  _model_root_input(model_root_input)
{
}

/**
 *
 */
void P2_INIT_PositionModelHitBoxes::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  system->update_input_hitboxes(_model_root_input);
  PT(ParticleSystem2::InputHitBoxCache) hbox_cache = system->_input_hitboxes[_model_root_input];

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    int hitbox = (int)(p2_normalized_rand() * (hbox_cache->_hitboxes.size() - 1));
    ParticleSystem2::HitBoxInfo &hbox = hbox_cache->_hitboxes[hitbox];
    p->_pos[0] = p2_random_min_max(hbox._ps_mins[0], hbox._ps_maxs[0]);
    p->_pos[1] = p2_random_min_max(hbox._ps_mins[1], hbox._ps_maxs[1]);
    p->_pos[2] = p2_random_min_max(hbox._ps_mins[2], hbox._ps_maxs[2]);
  }
}

BAM_WRITE(P2_INIT_PositionModelHitBoxes) {
  me.add_int8(_model_root_input);
}
BAM_READ(P2_INIT_PositionModelHitBoxes) {
  _model_root_input = scan.get_int8();
}
BAM_READ_FACTORY(P2_INIT_PositionModelHitBoxes);


/**
 *
 */
P2_INIT_AnimationIndexRandom::
P2_INIT_AnimationIndexRandom(int index_min, int index_max) :
  _anim_index_min(index_min),
  _anim_index_range(index_max - index_min)
{
}

/**
 *
 */
void P2_INIT_AnimationIndexRandom::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_anim_index = p2_random_min_range(_anim_index_min, _anim_index_range);
  }
}

BAM_WRITE(P2_INIT_AnimationIndexRandom) {
  me.add_int8(_anim_index_min);
  me.add_int8(_anim_index_range);
}
BAM_READ(P2_INIT_AnimationIndexRandom) {
  _anim_index_min = scan.get_int8();
  _anim_index_range = scan.get_int8();
}
BAM_READ_FACTORY(P2_INIT_AnimationIndexRandom);
