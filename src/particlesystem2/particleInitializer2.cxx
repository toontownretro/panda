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
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_pos = _point;
  }
}

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
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    p->_pos[0] = p2_random_min_max(_mins[0], _maxs[0]);
    p->_pos[1] = p2_random_min_max(_mins[1], _maxs[1]);
    p->_pos[2] = p2_random_min_max(_mins[2], _maxs[2]);
  }
}

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

    p->_pos = _center + vec;
  }
}

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
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    // Pick random point along line segment.
    PN_stdfloat frac = p2_normalized_rand();
    p->_pos = _a * (1.0f - frac) + _b * frac;
  }
}

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

  PN_stdfloat max_t = _curve->get_max_t();
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    // Random parametric point along curve.
    PN_stdfloat t = p2_normalized_rand() * max_t;
    // Evaluate position on curve at parametric point.
    _curve->get_point(t, p->_pos);
  }
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
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_velocity += _vel * p2_random_min_range(_amplitude_min, _amplitude_range, _amplitude_exponent);
  }
}

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
    p->_velocity += q.get_forward() * amplitude;
  }
}

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
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    LVector3 vec = p->_pos - _point;
    if (!vec.normalize()) {
      // Arbitrary direction.
      vec = LVector3::up();
    }

    PN_stdfloat amplitude = p2_random_min_range(_min_amplitude, _amplitude_range, _amplitude_exponent);

    p->_velocity += vec * amplitude;
  }
}

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
