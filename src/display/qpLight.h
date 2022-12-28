/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLight.h
 * @author brian
 * @date 2022-12-21
 */

#ifndef QPLIGHT_H
#define QPLIGHT_H

#include "pandabase.h"
#include "luse.h"
#include "typedReferenceCount.h"
#include "deg_2_rad.h"

/**
 *
 */
class EXPCL_PANDA_DISPLAY qpLight : public TypedReferenceCount {
  DECLARE_CLASS(qpLight, TypedReferenceCount);

PUBLISHED:
  enum Type {
    T_point,
    T_spot,
  };

  qpLight(Type type);

  INLINE void set_color_linear(const LVecBase3 &color);
  INLINE void set_color_srgb(const LVecBase3 &color);
  INLINE void set_color_srgb255_scalar(const LVecBase4 &color);
  INLINE const LVecBase3 &get_color_linear() const;

  INLINE void set_attenuation(PN_stdfloat constant, PN_stdfloat linear, PN_stdfloat quadratic);
  INLINE PN_stdfloat get_constant_atten() const { return _constant_atten; }
  INLINE PN_stdfloat get_linear_atten() const { return _linear_atten; }
  INLINE PN_stdfloat get_quadratic_atten() const { return _quadratic_atten; }

  INLINE void set_attenuation_radius(PN_stdfloat radius) { _atten_radius = radius; }
  INLINE PN_stdfloat get_attenuation_radius() const { return _atten_radius; }

  INLINE void set_cull_radius(PN_stdfloat radius) { _cull_radius = radius; }

  PN_stdfloat get_cull_radius() const;

  INLINE void set_pos(const LPoint3 &pos) { _pos = pos; }
  INLINE const LPoint3 &get_pos() const;

  INLINE void set_direction(const LVector3 &dir) { _direction = dir; }
  INLINE const LVector3 &get_direction() const { return _direction; }
  INLINE void set_hpr(const LVecBase3 &hpr);
  INLINE void set_quat(const LQuaternion &quat);

  INLINE void set_inner_cone(PN_stdfloat angle) { _inner_cone = deg_2_rad(angle); }
  INLINE PN_stdfloat get_inner_cone() const { return _inner_cone; }
  INLINE void set_outer_cone(PN_stdfloat angle) { _outer_cone = deg_2_rad(angle); }
  INLINE PN_stdfloat get_outer_cone() const { return _outer_cone; }
  INLINE void set_exponent(PN_stdfloat exp) { _exponent = exp; }
  INLINE PN_stdfloat get_exponent() const { return _exponent; }

  INLINE void set_light_type(Type type) { _light_type = type; }
  INLINE Type get_light_type() const { return _light_type; }

private:
  Type _light_type;

  // Floating point color of light in linear space.
  LVecBase3 _linear_color;

  PN_stdfloat _constant_atten;
  PN_stdfloat _linear_atten;
  PN_stdfloat _quadratic_atten;

  // Distance from light at which attenuation should drop off to 0.
  // Without this and just the attenuation coefficients above, the
  // light's sphere of influence is infinite.
  PN_stdfloat _atten_radius;

  // This is a hard limit on the culling radius of the light, without
  // affecting the attenuation equation.
  PN_stdfloat _cull_radius;

  // Currently world-space, unless I decide to make lights either be nodes
  // or be attached to nodes.
  LPoint3 _pos;
  LVector3 _direction;

  // Spotlight params.
  PN_stdfloat _inner_cone;
  PN_stdfloat _outer_cone;
  PN_stdfloat _exponent;
};

#include "qpLight.I"

#endif // QPLIGHT_H
