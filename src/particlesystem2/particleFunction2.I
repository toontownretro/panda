/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleFunction2.I
 * @author brian
 * @date 2022-04-04
 */

/**
 *
 */
INLINE ParticleLerpSegment::
ParticleLerpSegment() :
  _type(LT_constant),
  _start_frac(0.0f),
  _end_frac(1.0f),
  _start_value(0.0f),
  _end_value(0.0f),
  _scale_on_initial(false),
  _start_is_initial(false),
  _end_is_initial(false)
{
  _func_data[0] = 0.0f;
  _func_data[1] = 0.0f;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_range(float start, float end) {
  set_start(start);
  set_end(end);
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_start(float start) {
  _start_frac = start;
}

/**
 *
 */
INLINE float ParticleLerpSegment::
get_start() const {
  return _start_frac;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_end(float end) {
  _end_frac = end;
}

/**
 *
 */
INLINE float ParticleLerpSegment::
get_end() const {
  return _end_frac;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_type(LerpType type) {
  _type = type;

  switch (_type) {
  case LT_exponential:
    set_exponent(1.0f);
    break;
  case LT_stepwave:
    set_step_start_width(0.5f);
    set_step_end_width(0.5f);
    break;
  case LT_sinusoid:
    set_period(1.0f);
    break;
  default:
    break;
  }
}

/**
 *
 */
INLINE ParticleLerpSegment::LerpType ParticleLerpSegment::
get_type() const {
  return _type;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_start_value(const LVecBase3 &value) {
  _start_value = value;
}

/**
 *
 */
INLINE const LVecBase3 &ParticleLerpSegment::
get_start_value() const {
  return _start_value;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_end_value(const LVecBase3 &value) {
  _end_value = value;
}

/**
 *
 */
INLINE const LVecBase3 &ParticleLerpSegment::
get_end_value() const {
  return _end_value;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_scale_on_initial(bool flag) {
  _scale_on_initial = flag;
}

/**
 *
 */
INLINE bool ParticleLerpSegment::
is_scale_on_initial() const {
  return _scale_on_initial;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_start_is_initial(bool flag) {
  _start_is_initial = flag;
}

/**
 *
 */
INLINE bool ParticleLerpSegment::
get_start_is_initial() const {
  return _start_is_initial;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_end_is_initial(bool flag) {
  _end_is_initial = flag;
}

/**
 *
 */
INLINE bool ParticleLerpSegment::
get_end_is_initial() const {
  return _end_is_initial;
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_exponent(float exp) {
  nassertv(_type == LT_exponential);
  _func_data[0] = exp;
}

/**
 *
 */
INLINE float ParticleLerpSegment::
get_exponent() const {
  nassertr(_type == LT_exponential, 0.0f);
  return _func_data[0];
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_period(float period) {
  nassertv(_type == LT_sinusoid);
  _func_data[0] = period;
}

/**
 *
 */
INLINE float ParticleLerpSegment::
get_period() const {
  nassertr(_type == LT_sinusoid, 0.0f);
  return _func_data[0];
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_step_start_width(float width) {
  nassertv(_type == LT_stepwave);
  _func_data[0] = width;
}

/**
 *
 */
INLINE float ParticleLerpSegment::
get_step_start_width() const {
  nassertr(_type == LT_stepwave, 0.0f);
  return _func_data[0];
}

/**
 *
 */
INLINE void ParticleLerpSegment::
set_step_end_width(float width) {
  nassertv(_type == LT_stepwave);
  _func_data[1] = width;
}

/**
 *
 */
INLINE float ParticleLerpSegment::
get_step_end_width() const {
  nassertr(_type == LT_stepwave, 0.0f);
  return _func_data[1];
}
