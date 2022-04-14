/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_avx_src.I
 * @author brian
 * @date 2022-04-13
 */

/**
 * Fills all components of the float vector/SIMD register with a single
 * float value.
 */
ALWAYS_INLINE EightFloats::
EightFloats(float fill) {
  _data = _mm256_set1_ps(fill);
}

/**
 * Fills the float vector/SIMD register with the given separate float
 * values for each component.
 */
ALWAYS_INLINE EightFloats::
EightFloats(float a, float b, float c, float d,
            float e, float f, float g, float h) {
  _data = _mm256_set_ps(a, b, c, d, e, f, g, h);
}

/**
 *
 */
ALWAYS_INLINE EightFloats::
EightFloats(const float *data, bool aligned) {
  if (aligned) {
    _data = _mm256_load_ps(data);
  } else {
    _data = _mm256_loadu_ps(data);
  }
}

/**
 *
 */
ALWAYS_INLINE EightFloats::
EightFloats(const PN_vec8f &data) :
  _data(data)
{
}

/**
 *
 */
ALWAYS_INLINE EightFloats::
EightFloats(PN_vec8f &&data) :
  _data(std::move(data))
{
}

/**
 *
 */
ALWAYS_INLINE EightFloats::
EightFloats(const EightFloats &other) :
  _data(other._data)
{
}

/**
 *
 */
ALWAYS_INLINE EightFloats::
EightFloats(EightFloats &&other) :
  _data(std::move(other._data))
{
}

/**
 * Assuming that the vector is not already in an SIMD register, loads the
 * current values of the vector into an SIMD register, and stores the new
 * vector on this object.
 */
ALWAYS_INLINE void EightFloats::
load() {
#ifndef __GNUC__
  _data = _mm256_load_ps(_data.m256_f32);
#else
  _data = _mm256_load_ps(reinterpret_cast<const float *>(&_data));
#endif

}

/**
 * Loads a single value into all components of an SIMD register,
 * and stores the new vector on this object.
 */
ALWAYS_INLINE void EightFloats::
load(float fill) {
  _data = _mm256_set1_ps(fill);
}

/**
 * Loads four separate float values into an SIMD register, and stores
 * the new vector on this object.
 */
ALWAYS_INLINE void EightFloats::
load(float a, float b, float c, float d,
     float e, float f, float g, float h) {
  _data = _mm256_set_ps(a, b, c, d, e, f, g, h);
}

/**
 * Loads four floats from the given contiguous array of floats into an
 * SIMD register, and stores the new vector on this object.
 *
 * This version assumes the float array is aligned to 16-byte boundaries.
 * Use load_unaligned() for unaligned loads.
 */
ALWAYS_INLINE void EightFloats::
load(const float *data) {
  _data = _mm256_load_ps(data);
}

/**
 * Loads four floats from the given contiguous array of floats into an
 * SIMD register, and stores the new vector on this object.
 *
 * This version assumes the float array is not aligned to 16-byte boundaries.
 * Use load() for aligned loads.
 */
ALWAYS_INLINE void EightFloats::
load_unaligned(const float *data) {
  _data = _mm256_loadu_ps(data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator * (const EightFloats &other) const {
  return _mm256_mul_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator / (const EightFloats &other) const {
  return _mm256_div_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator - (const EightFloats &other) const {
  return _mm256_sub_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator + (const EightFloats &other) const {
  return _mm256_add_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator & (const EightFloats &other) const {
  return _mm256_and_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator | (const EightFloats &other) const {
  return _mm256_or_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator ^ (const EightFloats &other) const {
  return _mm256_xor_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator - () const {
  return _mm256_sub_ps(_zero._data, _data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator *= (const EightFloats &other) {
  _data = _mm256_mul_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator /= (const EightFloats &other) {
  _data = _mm256_div_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator -= (const EightFloats &other) {
  _data = _mm256_sub_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator += (const EightFloats &other) {
  _data = _mm256_add_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator &= (const EightFloats &other) {
  _data = _mm256_and_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator |= (const EightFloats &other) {
  _data = _mm256_or_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator ^= (const EightFloats &other) {
  _data = _mm256_xor_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator = (const EightFloats &other) {
  _data = other._data;
}

/**
 *
 */
ALWAYS_INLINE void EightFloats::
operator = (EightFloats &&other) {
  _data = std::move(other._data);
}

/**
 *
 */
ALWAYS_INLINE float EightFloats::
operator [] (int n) const {
#ifdef __GNUC__
  return (reinterpret_cast<const float *>(&_data))[n];
#else
  return _data.m256_f32[n];
#endif
}

ALWAYS_INLINE float &EightFloats::
operator [] (int n) {
#ifdef __GNUC__
  return (reinterpret_cast<float *>(&_data))[n];
#else
  return _data.m256_f32[n];
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator > (const EightFloats &other) const {
  return _mm256_cmp_ps(_data, other._data, _CMP_GT_OQ);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator >= (const EightFloats &other) const {
  return _mm256_cmp_ps(_data, other._data, _CMP_GE_OQ);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator < (const EightFloats &other) const {
  return _mm256_cmp_ps(_data, other._data, _CMP_LT_OQ);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator <= (const EightFloats &other) const {
  return _mm256_cmp_ps(_data, other._data, _CMP_LE_OQ);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator == (const EightFloats &other) const {
  return _mm256_cmp_ps(_data, other._data, _CMP_EQ_OQ);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
operator != (const EightFloats &other) const {
  return _mm256_cmp_ps(_data, other._data, _CMP_NEQ_OQ);
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_zero() const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, _zero._data, _CMP_EQ_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_negative() const {
  return _mm256_movemask_ps(_data) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_greater(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GT_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_greater_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GE_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_less(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LT_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_less_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LE_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_EQ_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_not_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_NEQ_OQ)) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_zero() const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_zero._data, _data, _CMP_EQ_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_negative() const {
  return _mm256_movemask_ps(_data) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_greater(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GT_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_greater_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GE_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_less(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LT_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_less_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LE_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_EQ_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_not_equal(const EightFloats &other) const {
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_NEQ_OQ)) == 0xFF;
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
min(const EightFloats &other) const {
  return _mm256_min_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
max(const EightFloats &other) const {
  return _mm256_max_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
sqrt() const {
  return _mm256_sqrt_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
rsqrt() const {
  return _mm256_rsqrt_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats EightFloats::
recip() const {
  return _mm256_rcp_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
zero() {
  return _zero;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
one() {
  return _one;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
negative_one() {
  return _negative_one;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
two() {
  return _two;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
three() {
  return _three;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
four() {
  return _four;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
point_five() {
  return _point_five;
}

/**
 *
 */
ALWAYS_INLINE const EightFloats &EightFloats::
flt_epsilon() {
  return _flt_epsilon;
}

/**
 *
 */
ALWAYS_INLINE EightFloats
simd_min(const EightFloats &a, const EightFloats &b) {
  return _mm256_min_ps(a._data, b._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats
simd_max(const EightFloats &a, const EightFloats &b) {
  return _mm256_max_ps(a._data, b._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats
simd_sqrt(const EightFloats &val) {
  return _mm256_sqrt_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats
simd_rsqrt(const EightFloats &val) {
  return _mm256_rsqrt_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE EightFloats
simd_recip(const EightFloats &val) {
  return _mm256_rcp_ps(val._data);
}
