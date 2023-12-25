/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleConstraint2.cxx
 * @author brian
 * @date 2022-04-11
 */

#include "particleConstraint2.h"
#include "particleSystem2.h"
#include "luse.h"
#include "p2_utils.h"

IMPLEMENT_CLASS(ParticleConstraint2);
IMPLEMENT_CLASS(PathParticleConstraint);
IMPLEMENT_CLASS(CollisionParticleConstraint);

/**
 *
 */
PathParticleConstraint::
PathParticleConstraint() :
  _min_distance(0.0f),
  _max_distance(100.0f),
  _max_distance_mid(-1.0f),
  _max_distance_end(-1.0f),
  _travel_time(10.0f),
  _random_bulge(0.0f),
  _start_input(0),
  _end_input(1),
  _bulge_control(0),
  _mid_point(0.5f)
{
}

/**
 *
 */
void
evaluate_path_points(int start_input, int end_input, PN_stdfloat mid, int bulge_control,
                     PN_stdfloat bulge,
                     double time, LPoint3 &start_pt, LPoint3 &mid_pt, LPoint3 &end_pt,
                     ParticleSystem2 *system) {
  start_pt = system->get_input_value(start_input)->get_pos();
  end_pt = system->get_input_value(end_input)->get_pos();
  mid_pt = start_pt + (end_pt - start_pt) * mid;

  if (bulge_control) {
    LVector3 target = end_pt - start_pt;
    PN_stdfloat bulge_scale = 0.0f;
    int input = start_input;
    if (bulge_control == 2) {
      input = end_input;
    }
    LVector3 fwd = system->get_input_value(input)->get_quat().get_forward();
    PN_stdfloat len = target.length();
    if (len > 1.0e-6) {
      target *= 1.0f / len;
      bulge_scale = 1.0f - std::abs(target.dot(fwd));
    }

    LPoint3 potential_mid_pt = fwd;
    PN_stdfloat offset_dist = potential_mid_pt.length();
    if (offset_dist > 1.0e-6) {
      potential_mid_pt *= (bulge * len * bulge_scale) / offset_dist;
      mid_pt += potential_mid_pt;
    }

  } else {
    // Offset mid point by random bulge vector.
    mid_pt += p2_random_unit_vector() * p2_random_min_max(-bulge, bulge);
  }
}

/**
 *
 */
bool PathParticleConstraint::
enforce_constraint(double time, double dt, ParticleSystem2 *system) {
  // Evaulate current constraint path.
  LPoint3 start_pt, mid_pt, end_pt;
  evaluate_path_points(_start_input, _end_input, _mid_point, _bulge_control,
                       _random_bulge, time, start_pt, mid_pt, end_pt, system);

  double timescale = 1.0 / std::max((double)0.001, (double)_travel_time);

  bool constant_radius = true;
  PN_stdfloat rad0 = _max_distance;
  PN_stdfloat radm = rad0;

  if (_max_distance_mid >= 0.0f) {
    constant_radius = _max_distance_mid == _max_distance;
    radm = _max_distance_mid;
  }

  PN_stdfloat rad1 = radm;
  if (_max_distance_end >= 0.0f) {
    constant_radius &= _max_distance_end == _max_distance;
    rad1 = _max_distance_end;
  }

  PN_stdfloat radm_minus_rad0 = radm - rad0;
  PN_stdfloat rad1_minus_radm = rad1 - radm;

  PN_stdfloat min_dist = _min_distance;
  PN_stdfloat min_dist_sqr = _min_distance * _min_distance;

  PN_stdfloat max_dist = std::max(rad0, std::max(radm, rad1));
  PN_stdfloat max_dist_sqr = max_dist * max_dist;

  bool changed_something = false;

  LVector3 delta0 = mid_pt - start_pt;
  LVector3 delta1 = end_pt - mid_pt;

  for (Particle &p : system->_particles) {
    if (!p._alive) {
      continue;
    }

    PN_stdfloat t_scale = std::min(1.0, timescale * (time - p._spawn_time));

    // Bezier
    LVector3 l0 = delta0;
    l0 *= t_scale;
    l0 += start_pt;

    LVector3 l1 = delta1;
    l1 *= t_scale;
    l1 += mid_pt;

    LVector3 center = l1;
    center -= l0;
    center *= t_scale;
    center += l0;

    LPoint3 point = p._pos;
    point -= center;

    PN_stdfloat dist_sqr = point.length_squared();
    bool too_far = dist_sqr > max_dist_sqr;
    if (!constant_radius && !too_far) {
      PN_stdfloat r0 = rad0 + (radm_minus_rad0 * t_scale);
      PN_stdfloat r1 = radm + (rad1_minus_radm * t_scale);
      max_dist = r0 + ((r1 - r0) * t_scale);

      too_far = dist_sqr > (max_dist * max_dist);
    }

    bool too_close = dist_sqr < min_dist_sqr;
    bool need_adjust = too_far || too_close;

    if (need_adjust) {
      //if ()

      PN_stdfloat guess = 1.0f / std::sqrt(dist_sqr);
      guess *= (3.0f - (dist_sqr * (guess * guess)));
      guess *= 0.5f;
      point *= guess;

      LPoint3 clamp_far = point;
      clamp_far *= max_dist;
      clamp_far += center;
      LPoint3 clamp_near = point;
      clamp_near *= min_dist;
      clamp_near += center;

      if (too_close) {
        p._pos = clamp_near;
      } else if (too_far) {
        p._pos = clamp_far;
      }
      changed_something = true;
    }
  }

  return changed_something;
}

/**
 *
 */
void PathParticleConstraint::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_int8(_start_input);
  me.add_int8(_end_input);
  me.add_stdfloat(_random_bulge);
  me.add_int8(_bulge_control);
  me.add_stdfloat(_mid_point);
  me.add_stdfloat(_travel_time);
  me.add_stdfloat(_min_distance);
  me.add_stdfloat(_max_distance);
  me.add_stdfloat(_max_distance_mid);
  me.add_stdfloat(_max_distance_end);
}

/**
 *
 */
void PathParticleConstraint::
fillin(DatagramIterator &scan, BamReader *manager) {
  _start_input = scan.get_int8();
  _end_input = scan.get_int8();
  _random_bulge = scan.get_stdfloat();
  _bulge_control = scan.get_int8();
  _mid_point = scan.get_stdfloat();
  _travel_time = scan.get_stdfloat();
  _min_distance = scan.get_stdfloat();
  _max_distance = scan.get_stdfloat();
  _max_distance_mid = scan.get_stdfloat();
  _max_distance_end = scan.get_stdfloat();
}

/**
 *
 */
TypedWritable *PathParticleConstraint::
make_from_bam(const FactoryParams &params) {
  PathParticleConstraint *obj = new PathParticleConstraint;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void PathParticleConstraint::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
CollisionParticleConstraint::
CollisionParticleConstraint() :
  _slide(0.0f),
  _bounce(0.5f),
  _accuracy_tolerance(1.0f),
  _kill_on_collision(false),
  _radius_scale(1.0f)
{
}

/**
 *
 */
bool CollisionParticleConstraint::
enforce_constraint(double time, double dt, ParticleSystem2 *system) {
  if (system->_tracer == nullptr) {
    return false;
  }

  bool bounce_or_slide = (_bounce != 0.0f) || (_slide != 0.0f);

  bool changed = false;

  for (Particle &p : system->_particles) {
    if (!p._alive || p._velocity.length_squared() <= 0.1f) {
      p._velocity.set(0, 0, 0);
      continue;
    }

    PN_stdfloat radius_factor = std::max(p._scale[0], p._scale[1]) * _radius_scale;

    LVector3 delta = p._pos - p._prev_pos;
    LVector3 delta_norm = delta;
    if (!delta_norm.normalize()) {
      continue;
    }
    LPoint3 end_point = p._pos + delta_norm * radius_factor;

    TraceInterface::TraceResult tr = system->_tracer->trace_line(p._prev_pos, end_point, system->_trace_mask);
    if (tr.has_hit()) {
      changed = true;

      PN_stdfloat frac = tr.get_frac();
      PN_stdfloat leftover_frac = tr.get_starts_solid() ? 0.0f : 1.0f - frac;

      LPoint3 new_point = p._prev_pos + delta * frac;

      if (bounce_or_slide) {
        LVector3 bounce = 2.0f * tr.get_surface_normal() * tr.get_surface_normal().dot(p._velocity) - p._velocity;
        bounce *= _bounce;
        LVector3 new_vel = -bounce;
        //bounce *= leftover_frac;
        new_point -= bounce * dt;

        /* LVector3 slide = tr.get_surface_normal() * tr.get_surface_normal().dot(delta) - delta;
        slide *= _slide;
        new_vel += slide;
        slide *= leftover_frac;
        new_point += slide; */

        //p._prev_pos = new_point - (new_vel * dt);
        p._velocity = new_vel;
      }

      p._pos = new_point;
    }
  }

  return changed;
}

/**
 *
 */
void CollisionParticleConstraint::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_stdfloat(_slide);
  me.add_stdfloat(_bounce);
  me.add_stdfloat(_accuracy_tolerance);
  me.add_stdfloat(_radius_scale);
  me.add_bool(_kill_on_collision);
}

/**
 *
 */
void CollisionParticleConstraint::
fillin(DatagramIterator &scan, BamReader *manager) {
  _slide = scan.get_stdfloat();
  _bounce = scan.get_stdfloat();
  _accuracy_tolerance = scan.get_stdfloat();
  _radius_scale = scan.get_stdfloat();
  _kill_on_collision = scan.get_bool();
}

/**
 *
 */
TypedWritable *CollisionParticleConstraint::
make_from_bam(const FactoryParams &params) {
  CollisionParticleConstraint *obj = new CollisionParticleConstraint;

  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  obj->fillin(scan, manager);
  return obj;
}

/**
 *
 */
void CollisionParticleConstraint::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}
