/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleConstraint2.h
 * @author brian
 * @date 2022-04-11
 */

#ifndef PARTICLECONSTRAINT2_H
#define PARTICLECONSTRAINT2_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"

class ParticleSystem2;
class FactoryParams;

/**
 * Base class for a physics contraint that applies to all particles in a
 * system.  Contraints are used to place limits on the movement of particles.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleConstraint2 : public TypedWritableReferenceCount {
  DECLARE_CLASS(ParticleConstraint2, TypedWritableReferenceCount);

PUBLISHED:
  ParticleConstraint2() = default;

public:
  virtual bool enforce_constraint(double time, double dt, ParticleSystem2 *system) = 0;
};

/**
 * Keeps particles constrained to a certain distance from a user-defined path.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 PathParticleConstraint : public ParticleConstraint2 {
  DECLARE_CLASS(PathParticleConstraint, ParticleConstraint2);

PUBLISHED:
  PathParticleConstraint();

  INLINE void set_start_input(int input);
  INLINE int get_start_input() const;
  MAKE_PROPERTY(start_input, get_start_input, set_start_input);

  INLINE void set_end_input(int input);
  INLINE int get_end_input() const;
  MAKE_PROPERTY(end_input, get_end_input, set_end_input);

  INLINE void set_bulge_control(int control);
  INLINE int get_bulge_control() const;
  MAKE_PROPERTY(bulge_control, get_bulge_control, set_bulge_control);

  INLINE void set_random_bulge(PN_stdfloat amount);
  INLINE PN_stdfloat get_random_bulge() const;
  MAKE_PROPERTY(random_bulge, get_random_bulge, set_random_bulge);

  INLINE void set_mid_point(PN_stdfloat mid);
  INLINE PN_stdfloat get_mid_point() const;
  MAKE_PROPERTY(mid_point, get_mid_point, set_mid_point);

  INLINE void set_travel_time(PN_stdfloat time);
  INLINE PN_stdfloat get_travel_time() const;
  MAKE_PROPERTY(travel_time, get_travel_time, set_travel_time);

  INLINE void set_min_distance(PN_stdfloat dist);
  INLINE PN_stdfloat get_min_distance() const;
  MAKE_PROPERTY(min_distance, get_min_distance, set_min_distance);

  INLINE void set_max_distance(PN_stdfloat dist);
  INLINE PN_stdfloat get_max_distance() const;
  MAKE_PROPERTY(max_distance, get_max_distance, set_max_distance);

  INLINE void set_max_distance_mid(PN_stdfloat dist);
  INLINE PN_stdfloat get_max_distance_mid() const;
  MAKE_PROPERTY(max_distance_mid, get_max_distance_mid, set_max_distance_mid);

  INLINE void set_max_distance_end(PN_stdfloat dist);
  INLINE PN_stdfloat get_max_distance_end() const;
  MAKE_PROPERTY(max_distance_end, get_max_distance_end, set_max_distance_end);

public:
  virtual bool enforce_constraint(double time, double dt, ParticleSystem2 *system) override;

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  int _start_input, _end_input;
  PN_stdfloat _random_bulge;
  int _bulge_control;
  PN_stdfloat _mid_point;

  PN_stdfloat _travel_time;
  PN_stdfloat _min_distance;
  PN_stdfloat _max_distance, _max_distance_mid, _max_distance_end;
};

/**
 * Makes particles collide with the scene.  Requires the system/manager to be given a
 * valid TraceInterface pointer.
 */
class EXPCL_PANDA_PARTICLESYSTEM2 CollisionParticleConstraint : public ParticleConstraint2 {
  DECLARE_CLASS(CollisionParticleConstraint, ParticleConstraint2);

PUBLISHED:
  CollisionParticleConstraint();

  INLINE void set_slide(PN_stdfloat slide) { _slide = slide; }
  INLINE PN_stdfloat get_slide() const { return _slide; }
  MAKE_PROPERTY(slide, get_slide, set_slide);

  INLINE void set_bounce(PN_stdfloat bounce) { _bounce = bounce; }
  INLINE PN_stdfloat get_bounce() const { return _bounce; }
  MAKE_PROPERTY(bounce, get_bounce, set_bounce);

  INLINE void set_accuracy_tolerance(PN_stdfloat tolerance) { _accuracy_tolerance = tolerance; }
  INLINE PN_stdfloat get_accuracy_tolerance() const { return _accuracy_tolerance; }
  MAKE_PROPERTY(accuracy_tolerance, get_accuracy_tolerance, set_accuracy_tolerance);

  INLINE void set_kill_on_collision(bool flag) { _kill_on_collision = flag; }
  INLINE bool get_kill_on_collision() const { return _kill_on_collision; }
  MAKE_PROPERTY(kill_on_collision, get_kill_on_collision, set_kill_on_collision);

  INLINE void set_radius_scale(PN_stdfloat scale) { _radius_scale = scale; }
  INLINE PN_stdfloat get_radius_scale() const { return _radius_scale; }
  MAKE_PROPERTY(radius_scale, get_radius_scale, set_radius_scale);

public:
  virtual bool enforce_constraint(double time, double dt, ParticleSystem2 *system) override;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  static void register_with_read_factory();
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  PN_stdfloat _slide;
  PN_stdfloat _bounce;
  PN_stdfloat _radius_scale;
  bool _kill_on_collision;
  PN_stdfloat _accuracy_tolerance;
};

#include "particleConstraint2.I"

#endif // PARTICLECONSTRAINT2_H
