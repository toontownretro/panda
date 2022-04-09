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

IMPLEMENT_CLASS(P2_INIT_LifespanRandom);

IMPLEMENT_CLASS(P2_INIT_PositionExplicit); // PointEmitter
IMPLEMENT_CLASS(P2_INIT_PositionBoxVolume); // BoxEmitter
IMPLEMENT_CLASS(P2_INIT_PositionRectangleArea); // RectangleEmitter
IMPLEMENT_CLASS(P2_INIT_PositionSphereVolume); // SphereVolumeEmitter
IMPLEMENT_CLASS(P2_INIT_PositionSphereSurface); // SphereSurfaceEmitter
IMPLEMENT_CLASS(P2_INIT_PositionCircleArea); // DiscEmitter
IMPLEMENT_CLASS(P2_INIT_PositionCirclePerimeter); // RingEmitter
//IMPLEMENT_CLASS(P2_INIT_PositionArc); // ArcEmitter
IMPLEMENT_CLASS(P2_INIT_PositionLineSegment); // LineEmitter
IMPLEMENT_CLASS(P2_INIT_PositionParametricCurve);
IMPLEMENT_CLASS(P2_INIT_PositionCharacterJoints); //

IMPLEMENT_CLASS(P2_INIT_VelocityExplicit); // ET_EXPLICIT
IMPLEMENT_CLASS(P2_INIT_VelocityCone); //
IMPLEMENT_CLASS(P2_INIT_VelocityRadiate); // ET_RADIATE

IMPLEMENT_CLASS(P2_INIT_RotationRandom);

//IMPLEMENT_CLASS(P2_INIT_ScaleRandom);

/**
 *
 */
P2_INIT_LifespanRandom::
P2_INIT_LifespanRandom(PN_stdfloat ls_min, PN_stdfloat ls_max) :
  _lifespan_min(ls_min),
  _lifespan_max(ls_max)
{
}

/**
 *
 */
void P2_INIT_LifespanRandom::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_duration = p2_random_min_max(_lifespan_min, _lifespan_max);
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
P2_INIT_PositionRectangleArea::
P2_INIT_PositionRectangleArea(const LPoint2 &a, const LPoint2 &b) :
  _a(a),
  _b(b)
{
}

/**
 *
 */
void P2_INIT_PositionRectangleArea::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_pos[0] = p2_random_min_max(_a[0], _b[0]);
    p->_pos[1] = p2_random_min_max(_a[1], _b[1]);
    p->_pos[2] = 0.0f;
  }
}

/**
 *
 */
P2_INIT_PositionSphereVolume::
P2_INIT_PositionSphereVolume(const LPoint3 &center, PN_stdfloat radius) :
  _center(center),
  _radius(radius)
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

    // Random distance along vector.
    PN_stdfloat dist = p2_normalized_rand() * _radius;

    p->_pos = _center + (vec * dist);
  }
}

/**
 *
 */
P2_INIT_PositionSphereSurface::
P2_INIT_PositionSphereSurface(const LPoint3 &center, PN_stdfloat radius_min, PN_stdfloat radius_max) :
  _center(center),
  _radius_min(radius_min),
  _radius_max(radius_max)
{
}

/**
 *
 */
void P2_INIT_PositionSphereSurface::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_pos = _center + p2_random_unit_vector() * p2_random_min_max(_radius_min, _radius_max);
  }
}

/**
 *
 */
P2_INIT_PositionCircleArea::
P2_INIT_PositionCircleArea(const LPoint2 &center, PN_stdfloat radius) :
  _center(center),
  _radius(radius)
{
}

/**
 *
 */
void P2_INIT_PositionCircleArea::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    PN_stdfloat dist = p2_normalized_rand() * _radius;
    PN_stdfloat theta = p2_normalized_rand() * 2.0f * MathNumbers::pi_f;
    p->_pos[0] = std::cos(theta) * dist;
    p->_pos[1] = std::sin(theta) * dist;
    p->_pos[2] = 0.0f;
  }
}

/**
 *
 */
P2_INIT_PositionCirclePerimeter::
P2_INIT_PositionCirclePerimeter(const LPoint2 &center, PN_stdfloat radius_min, PN_stdfloat radius_max) :
  _center(center),
  _radius_min(radius_max),
  _radius_max(radius_max)
{
}

/**
 *
 */
void P2_INIT_PositionCirclePerimeter::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    PN_stdfloat theta = p2_normalized_rand() * 2.0f * MathNumbers::pi_f;
    PN_stdfloat dist = p2_random_min_max(_radius_min, _radius_max);
    p->_pos[0] = std::cos(theta) * dist;
    p->_pos[1] = std::sin(theta) * dist;
    p->_pos[2] = 0.0f;
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
P2_INIT_VelocityExplicit(const LVector3 &dir, PN_stdfloat amp_min, PN_stdfloat amp_max) :
  _vel(dir),
  _amplitude_min(amp_min),
  _amplitude_range(amp_max - amp_min)
{
}

/**
 *
 */
void P2_INIT_VelocityExplicit::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];
    p->_velocity += _vel * p2_random_min_range(_amplitude_min, _amplitude_range);
  }
}

/**
 *
 */
P2_INIT_VelocityCone::
P2_INIT_VelocityCone(const LVecBase3 &min_hpr, const LVecBase3 &max_hpr,
                     PN_stdfloat min_amplitude, PN_stdfloat max_amplitude) :
  _min_hpr(min_hpr),
  _max_hpr(max_hpr),
  _min_amplitude(min_amplitude),
  _max_amplitude(max_amplitude)
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
    PN_stdfloat amplitude = p2_random_min_max(_min_amplitude, _max_amplitude);

    // Construct velocity vector.
    p->_velocity += q.get_forward() * amplitude;
  }
}

/**
 *
 */
P2_INIT_VelocityRadiate::
P2_INIT_VelocityRadiate(const LPoint3 &point, PN_stdfloat min_amp, PN_stdfloat max_amp) :
  _point(point),
  _min_amplitude(min_amp),
  _max_amplitude(max_amp)
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

    PN_stdfloat amplitude = p2_random_min_max(_min_amplitude, _max_amplitude);

    p->_velocity += vec * amplitude;
  }
}

/**
 *
 */
P2_INIT_RotationRandom::
P2_INIT_RotationRandom(PN_stdfloat min_rot, PN_stdfloat max_rot,
                       PN_stdfloat min_rot_speed, PN_stdfloat max_rot_speed) :
  _min_rot(min_rot),
  _max_rot(max_rot),
  _min_rot_speed(min_rot_speed),
  _max_rot_speed(max_rot_speed)
{
}

/**
 *
 */
void P2_INIT_RotationRandom::
init_particles(double time, int *particles, int num_particles, ParticleSystem2 *system) {
  PN_stdfloat rot_range = _max_rot - _min_rot;
  PN_stdfloat rot_speed_range = _max_rot_speed - _min_rot_speed;

  for (int i = 0; i < num_particles; ++i) {
    Particle *p = &system->_particles[particles[i]];

    p->_rotation = p2_random_min_range(_min_rot, rot_range);
    p->_rotation_speed = p2_random_min_range(_min_rot_speed, rot_speed_range);
  }
}
