/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_avx512_src.I
 * @author brian
 * @date 2022-04-13
 */

/**
 * Fills all components of the float vector/SIMD register with a single
 * float value.
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(float fill) {
  _data = _mm512_set1_ps(fill);
}

/**
 * Fills the float vector/SIMD register with the given separate float
 * values for each component.
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(float a, float b, float c, float d,
              float e, float f, float g, float h,
              float i, float j, float k, float l,
              float m, float n, float o, float p) {
  _data = _mm512_set_ps(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(const float *data, bool aligned) {
  if (aligned) {
    _data = _mm512_load_ps(data);
  } else {
    _data = _mm512_loadu_ps(data);
  }
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(const PN_vec16f &data) :
  _data(data)
{
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(PN_vec16f &&data) :
  _data(std::move(data))
{
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(const SixteenFloats &other) :
  _data(other._data)
{
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats::
SixteenFloats(SixteenFloats &&other) :
  _data(std::move(other._data))
{
}

/**
 * Assuming that the vector is not already in an SIMD register, loads the
 * current values of the vector into an SIMD register, and stores the new
 * vector on this object.
 */
ALWAYS_INLINE void SixteenFloats::
load() {
#ifndef __GNUC__
  _data = _mm512_load_ps(_data.m512_f32);
#else
  _data = _mm512_load_ps(reinterpret_cast<const float *>(&_data));
#endif

}

/**
 * Loads a single value into all components of an SIMD register,
 * and stores the new vector on this object.
 */
ALWAYS_INLINE void SixteenFloats::
load(float fill) {
  _data = _mm512_set1_ps(fill);
}

/**
 * Loads sixteen separate float values into an SIMD register, and stores
 * the new vector on this object.
 */
ALWAYS_INLINE void SixteenFloats::
load(float a, float b, float c, float d,
     float e, float f, float g, float h,
     float i, float j, float k, float l,
     float m, float n, float o, float p) {
  _data = _mm512_set_ps(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
}

/**
 * Loads four floats from the given contiguous array of floats into an
 * SIMD register, and stores the new vector on this object.
 *
 * This version assumes the float array is aligned to 16-byte boundaries.
 * Use load_unaligned() for unaligned loads.
 */
ALWAYS_INLINE void SixteenFloats::
load(const float *data) {
  _data = _mm512_load_ps(data);
}

/**
 * Loads four floats from the given contiguous array of floats into an
 * SIMD register, and stores the new vector on this object.
 *
 * This version assumes the float array is not aligned to 16-byte boundaries.
 * Use load() for aligned loads.
 */
ALWAYS_INLINE void SixteenFloats::
load_unaligned(const float *data) {
  _data = _mm512_loadu_ps(data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator * (const SixteenFloats &other) const {
  return _mm512_mul_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator / (const SixteenFloats &other) const {
  return _mm512_div_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator - (const SixteenFloats &other) const {
  return _mm512_sub_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator + (const SixteenFloats &other) const {
  return _mm512_add_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator & (const SixteenFloats &other) const {
  return _mm512_and_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator | (const SixteenFloats &other) const {
  return _mm512_or_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator ^ (const SixteenFloats &other) const {
  return _mm512_xor_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
operator - () const {
  return _mm512_sub_ps(_zero._data, _data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator *= (const SixteenFloats &other) {
  _data = _mm512_mul_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator /= (const SixteenFloats &other) {
  _data = _mm512_div_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator -= (const SixteenFloats &other) {
  _data = _mm512_sub_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator += (const SixteenFloats &other) {
  _data = _mm512_add_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator &= (const SixteenFloats &other) {
  _data = _mm512_and_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator |= (const SixteenFloats &other) {
  _data = _mm512_or_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator ^= (const SixteenFloats &other) {
  _data = _mm512_xor_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator = (const SixteenFloats &other) {
  _data = other._data;
}

/**
 *
 */
ALWAYS_INLINE void SixteenFloats::
operator = (SixteenFloats &&other) {
  _data = std::move(other._data);
}

/**
 *
 */
ALWAYS_INLINE float SixteenFloats::
operator [] (int n) const {
#ifdef __GNUC__
  return (reinterpret_cast<const float *>(&_data))[n];
#else
  return _data.m512_f32[n];
#endif
}

ALWAYS_INLINE float &SixteenFloats::
operator [] (int n) {
#ifdef __GNUC__
  return (reinterpret_cast<float *>(&_data))[n];
#else
  return _data.m512_f32[n];
#endif
}

/**
 *
 */
ALWAYS_INLINE BitMask32 SixteenFloats::
operator > (const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_GT_OQ);
}

/**
 *
 */
ALWAYS_INLINE BitMask32 SixteenFloats::
operator >= (const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_GE_OQ);
}

/**
 *
 */
ALWAYS_INLINE BitMask32 SixteenFloats::
operator < (const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_LT_OQ);
}

/**
 *
 */
ALWAYS_INLINE BitMask32 SixteenFloats::
operator <= (const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_LE_OQ);
}

/**
 *
 */
ALWAYS_INLINE BitMask32 SixteenFloats::
operator == (const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ);
}

/**
 *
 */
ALWAYS_INLINE BitMask32 SixteenFloats::
operator != (const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ);
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_zero() const {
  return _mm512_cmp_ps_mask(_data, _zero._data, _CMP_EQ_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_negative() const {
  return _mm512_cmp_ps_mask(_data, _zero._data, _CMP_LT_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_greater(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_GT_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_greater_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_GE_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_less(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_LT_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_less_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_LE_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_any_not_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_zero() const {
  return _mm512_cmp_ps_mask(_zero._data, _data, _CMP_EQ_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_negative() const {
  return _mm512_cmp_ps_mask(_data, _zero._data, _CMP_LT_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_greater(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_GT_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_greater_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_GE_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_less(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_LT_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_less_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_LE_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE bool SixteenFloats::
is_all_not_equal(const SixteenFloats &other) const {
  return _mm512_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ) == 0xFFFF;
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
min(const SixteenFloats &other) const {
  return _mm512_min_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
max(const SixteenFloats &other) const {
  return _mm512_max_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
sqrt() const {
  return _mm512_sqrt_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
rsqrt() const {
  return _mm512_rsqrt(_data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats SixteenFloats::
recip() const {
  return _mm512_rcp_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
zero() {
  return _zero;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
one() {
  return _one;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
negative_one() {
  return _negative_one;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
two() {
  return _two;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
three() {
  return _three;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
four() {
  return _four;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
point_five() {
  return _point_five;
}

/**
 *
 */
ALWAYS_INLINE const SixteenFloats &SixteenFloats::
flt_epsilon() {
  return _flt_epsilon;
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats
simd_min(const SixteenFloats &a, const SixteenFloats &b) {
  return _mm512_min_ps(a._data, b._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats
simd_max(const SixteenFloats &a, const SixteenFloats &b) {
  return _mm512_max_ps(a._data, b._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats
simd_sqrt(const SixteenFloats &val) {
  return _mm512_sqrt_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats
simd_rsqrt(const SixteenFloats &val) {
  return _mm512_rsqrt_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE SixteenFloats
simd_recip(const SixteenFloats &val) {
  return _mm512_rcp_ps(val._data);
}
