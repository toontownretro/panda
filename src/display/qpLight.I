/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLight.I
 * @author brian
 * @date 2022-12-21
 */

/**
 *
 */
INLINE void qpLight::
set_color_linear(const LVecBase3 &color) {
  _linear_color = color;
}

/**
 *
 */
INLINE void qpLight::
set_color_srgb(const LVecBase3 &color) {
  _linear_color[0] = cpow(color[0], 2.2f);
  _linear_color[1] = cpow(color[1], 2.2f);
  _linear_color[2] = cpow(color[2], 2.2f);
}

/**
 *
 */
INLINE void qpLight::
set_color_srgb255_scalar(const LVecBase4 &color) {
  set_color_srgb(color.get_xyz() / 255.0f);
  _linear_color *= color[3] / 255.0f;
}

/**
 *
 */
INLINE const LVecBase3 &qpLight::
get_color_linear() const {
  return _linear_color;
}

/**
 *
 */
INLINE void qpLight::
set_attenuation(PN_stdfloat constant, PN_stdfloat linear, PN_stdfloat quadratic) {
  _constant_atten = constant;
  _linear_atten = linear;
  _quadratic_atten = quadratic;
}

/**
 *
 */
INLINE const LPoint3 &qpLight::
get_pos() const {
  return _pos;
}

/**
 *
 */
INLINE void qpLight::
set_quat(const LQuaternion &quat) {
  _direction = quat.get_forward();
}

/**
 *
 */
INLINE void qpLight::
set_hpr(const LVecBase3 &hpr) {
  LQuaternion quat;
  quat.set_hpr(hpr);
  _direction = quat.get_forward();
}
