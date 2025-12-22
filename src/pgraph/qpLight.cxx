/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLight.cxx
 * @author brian
 * @date 2022-12-21
 */

#include "qpLight.h"
#include "qpLightManager.h"

IMPLEMENT_CLASS(qpLight);

/**
 *
 */
qpLight::
qpLight(Type type) :
  _light_type(type),
  _linear_color(1.0f, 1.0f, 1.0f),
  _constant_atten(0.0f),
  _linear_atten(0.0f),
  _quadratic_atten(1.0f),
  _atten_radius(0.0f),
  _cull_radius(0.0f),
  _pos(0.0f, 0.0f, 0.0f),
  _direction(LVector3::forward()),
  _inner_cone(deg_2_rad(35.0f)),
  _outer_cone(deg_2_rad(45.0f)),
  _exponent(1.0f),
  _manager(nullptr)
{
}

/**
 *
 */
PN_stdfloat qpLight::
get_cull_radius() const {
  if (_cull_radius > 0.0f) {
    return _cull_radius;

  } else if (_atten_radius > 0.0f) {
    return _atten_radius;

  } else {
    PN_stdfloat a = _quadratic_atten;
    PN_stdfloat b = _linear_atten;
    PN_stdfloat c = _constant_atten;
    PN_stdfloat y = (1.0f / 256.0f) / std::max(_linear_color[0], std::max(_linear_color[1], _linear_color[2]));
    PN_stdfloat x = -b;
    x += csqrt(cabs(b * b - 4 * a * (c - 1.0f / y)));
    x /= 2 * a;
    return x;
  }
}

/**
 *
 */
void qpLight::
mark_dirty() {
  if (_manager != nullptr) {
    _manager->mark_dynamic_lights_dirty();
  }
}
