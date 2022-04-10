/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleEmitter2.cxx
 * @author brian
 * @date 2022-04-04
 */

#include "particleEmitter2.h"
#include "p2_utils.h"
#include "config_particlesystem2.h"

TypeHandle ParticleEmitter2::_type_handle;
TypeHandle BurstParticleEmitter::_type_handle;
TypeHandle ContinuousParticleEmitter::_type_handle;

/**
 *
 */
BurstParticleEmitter::
BurstParticleEmitter() :
  _start_time(0.0f),
  _litter_min(1),
  _litter_max(1)
{
}

/**
 *
 */
int BurstParticleEmitter::
update(double time) {
  return 0;
}

/**
 *
 */
ContinuousParticleEmitter::
ContinuousParticleEmitter() :
  _litter_min(1),
  _litter_max(1),
  _interval_min(1.0f),
  _interval_max(1.0f),
  _start_time(0.0f),
  _duration(0.0f),
  _last_litter(0.0f),
  _next_interval(0.0f)
{
}

/**
 *
 */
void ContinuousParticleEmitter::
set_emission_rate(PN_stdfloat particles_per_second) {
  // By giving an emission rate, we are birthing 1 particle
  // every N seconds with no randomization.

  _litter_min = 1;
  _litter_max = 1;

  _interval_min = 1.0f / particles_per_second;
  _interval_max = _interval_min;
}

/**
 *
 */
void ContinuousParticleEmitter::
set_interval_and_litter_size(PN_stdfloat interval_min, PN_stdfloat interval_max,
                             int litter_min, int litter_max) {
  _interval_min = interval_min;
  _interval_max = interval_max;
  _litter_min = litter_min;
  _litter_max = litter_max;
}

/**
 *
 */
void ContinuousParticleEmitter::
set_start_time(PN_stdfloat time) {
  _start_time = time;
}

/**
 *
 */
void ContinuousParticleEmitter::
set_duration(PN_stdfloat duration) {
  _duration = duration;
}

/**
 *
 */
int ContinuousParticleEmitter::
update(double time) {
  if (time < (double)_start_time) {
    // Not active yet.
    return 0;

  } else if (_duration > 0.0f) {
    // If emitter has a non-zero duration, the emitter
    // is only active on a timeslice of the system.
    double end_time = (double)_start_time + (double)_duration;
    if (time >= end_time) {
      return 0;
    }
  }

  int count = 0;
  double remaining_time = time - _last_litter;
  int litter_count = 0;
  while (remaining_time >= _next_interval) {
    count += (int)p2_random_min_max(_litter_min, _litter_max);
    remaining_time -= _next_interval;
    _next_interval = p2_random_min_max(_interval_min, _interval_max);
    litter_count++;
  }

  if (particlesystem2_cat.is_debug()) {
    particlesystem2_cat.debug()
      << litter_count << " liters on this update\n";
  }

  if (count > 0) {
    _last_litter = time;
  }

  return count;

  return 0;
}
