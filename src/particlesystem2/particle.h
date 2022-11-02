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
#include "internalName.h"

#define P2_MAX_PARTICLE_FIELDS 32

enum ParticleField : uint8_t {
  PF_position,
  PF_linear_velocity,
  PF_lifespan,
  PF_scale,
  PF_angles,
  PF_angular_velocity,
  PF_color_rgb,
  PF_alpha,
  PF_birth_time,
  PF_trail_length,

  PF_BUILTIN_COUNT,
};

enum ParticleNumericType : uint8_t {
  PNT_int8,
  PNT_uint8,
  PNT_int16,
  PNT_uint16,
  PNT_int32,
  PNT_uint32,
  PNT_int64,
  PNT_uint64,
  PNT_float32,
  PNT_float64,
};

class ParticleFieldDef {
public:
  CPT_InternalName _name;
  unsigned char _field_id;
  ParticleNumericType _numeric_type;
  unsigned char _num_components;

  INLINE bool is_integer() const { return _numeric_type <= PNT_uint64; }
  INLINE bool is_float() const { return _numeric_type >= PNT_float32; }
  INLINE unsigned char get_component_width() const;
  INLINE size_t get_stride() const { return get_component_width() * _num_components; }
};

/**
 * Data for a single particle in a particle system.
 */
class Particle {
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

  // For animated sprites.  The animation is stored in a texture array
  // where each page represents a frame of the animation.  It can also
  // store multiple animations in succession, and particles can choose
  // one random animation to play.

  // This specifies which animation the particle should use.
  int _anim_index;

  LPoint3 _initial_pos;
  LVector3 _initial_vel;
  LVecBase2 _initial_scale;
  LColor _initial_color;
  PN_stdfloat _initial_rotation;
  PN_stdfloat _initial_rotation_speed;
};

#include "particle.I"

#endif // PARTICLE_H
