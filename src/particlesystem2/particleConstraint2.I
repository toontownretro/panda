/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleConstraint2.I
 * @author brian
 * @date 2022-04-11
 */

/**
 *
 */
INLINE void PathParticleConstraint::
set_start_input(int input) {
  _start_input = input;
}

/**
 *
 */
INLINE int PathParticleConstraint::
get_start_input() const {
  return _start_input;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_end_input(int input) {
  _end_input = input;
}

/**
 *
 */
INLINE int PathParticleConstraint::
get_end_input() const {
  return _end_input;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_bulge_control(int control) {
  _bulge_control = control;
}

/**
 *
 */
INLINE int PathParticleConstraint::
get_bulge_control() const {
  return _bulge_control;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_random_bulge(PN_stdfloat bulge) {
  _random_bulge = bulge;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_random_bulge() const {
  return _random_bulge;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_mid_point(PN_stdfloat mid) {
  _mid_point = mid;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_mid_point() const {
  return _mid_point;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_travel_time(PN_stdfloat time) {
  _travel_time = time;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_travel_time() const {
  return _travel_time;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_min_distance(PN_stdfloat dist) {
  _min_distance = dist;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_min_distance() const {
  return _min_distance;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_max_distance(PN_stdfloat dist) {
  _max_distance = dist;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_max_distance() const {
  return _max_distance;
}
/**
 *
 */
INLINE void PathParticleConstraint::
set_max_distance_mid(PN_stdfloat dist) {
  _max_distance_mid = dist;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_max_distance_mid() const {
  return _max_distance_mid;
}

/**
 *
 */
INLINE void PathParticleConstraint::
set_max_distance_end(PN_stdfloat dist) {
  _max_distance_end = dist;
}

/**
 *
 */
INLINE PN_stdfloat PathParticleConstraint::
get_max_distance_end() const {
  return _max_distance_end;
}
