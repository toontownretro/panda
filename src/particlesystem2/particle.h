/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particle.h
 * @author brian
 * @date 2022-04-02
 */

#ifndef PARTICLE_H
#define PARTICLE_H

#include "pandabase.h"
#include "luse.h"

/**
 * Data for a single particle in a particle system.
 */
class Particle {
PUBLISHED:
public:
  LPoint3 _pos;
  LPoint3 _prev_pos;
  LVector3 _velocity;
  // How long particle should live for.
  PN_stdfloat _duration;
  LVecBase2 _scale;
  PN_stdfloat _rotation;
  PN_stdfloat _rotation_speed;
  LColor _color;
  PN_stdfloat _spawn_time;
  size_t _id;
  bool _alive;
};

#include "particle.I"

#endif // PARTICLE_H
