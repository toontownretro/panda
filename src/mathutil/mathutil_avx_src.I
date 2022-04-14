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
 *
 */
ALWAYS_INLINE EightFloatsMask::
EightFloatsMask(PN_vec8f_mask &&mask) :
  _mask(std::move(mask))
{
}

/**
 *
 */
ALWAYS_INLINE void EightFloatsMask::
operator = (PN_vec8f_mask &&mask) {
  _mask = std::move(mask);
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloatsMask::
operator & (const EightFloatsMask &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mask & other._mask;
#else
  return _mm256_and_ps(_mask, other._mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloatsMask::
operator | (const EightFloatsMask &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mask | other._mask;
#else
  return _mm256_or_ps(_mask, other._mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloatsMask::
operator ^ (const EightFloatsMask &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mask ^ other._mask;
#else
  return _mm256_xor_ps(_mask, other._mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloatsMask::
operator ~ () const {
#ifdef HAVE_MASK_REGISTERS
  return ~_mask;
#else
  return _mm256_xor_ps(EightFloats::_negative_one._data, _mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloatsMask::
is_all_on() const {
#ifdef HAVE_MASK_REGISTERS
  return _mask == 0xF;
#else
  return _mm256_movemask_ps(_mask) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloatsMask::
is_all_off() const {
#ifdef HAVE_MASK_REGISTERS
  return _mask == 0;
#else
  return _mm256_movemask_ps(_mask) == 0;
#endif
}

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
#if defined(__clang__) || defined(__GNUC__)
  _data = _mm256_load_ps(reinterpret_cast<const float *>(&_data));
#else
  _data = _mm256_load_ps(_data.m256_f32);
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
 * Shorthand for blend_zero().
 *
 * Does a masked blend with 0 on AVX-512, an actual AND otherwise.
 */
ALWAYS_INLINE EightFloats EightFloats::
operator & (const EightFloatsMask &mask) const {
  return blend_zero(mask);
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
ALWAYS_INLINE const PN_vec8f &EightFloats::
operator * () const {
  return _data;
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
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<const float *>(&_data))[n];
#else
  return _data.m256_f32[n];
#endif
}

ALWAYS_INLINE float &EightFloats::
operator [] (int n) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<float *>(&_data))[n];
#else
  return _data.m256_f32[n];
#endif
}

/**
 *
 */
ALWAYS_INLINE float *EightFloats::
modify_data() {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<float *>(&_data);
#else
  return _data.m256_f32;
#endif
}

/**
 *
 */
ALWAYS_INLINE const float *EightFloats::
get_data() const {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<const float *>(&_data);
#else
  return _data.m256_f32;
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloats::
operator > (const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_GT_OQ);
#else
  return _mm256_cmp_ps(_data, other._data, _CMP_GT_OQ);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloats::
operator >= (const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_GE_OQ);
#else
  return _mm256_cmp_ps(_data, other._data, _CMP_GE_OQ);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloats::
operator < (const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_LT_OQ);
#else
  return _mm256_cmp_ps(_data, other._data, _CMP_LT_OQ);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloats::
operator <= (const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_LE_OQ);
#else
  return _mm256_cmp_ps(_data, other._data, _CMP_LE_OQ);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloats::
operator == (const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ);
#else
  return _mm256_cmp_ps(_data, other._data, _CMP_EQ_OQ);
#endif
}

/**
 *
 */
ALWAYS_INLINE EightFloatsMask EightFloats::
operator != (const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ);
#else
  return _mm256_cmp_ps(_data, other._data, _CMP_NEQ_OQ);
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_zero() const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, _zero._data, _CMP_EQ_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, _zero._data, _CMP_EQ_OQ)) != 0;
#endif
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
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_GT_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GT_OQ)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_greater_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_GE_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GE_OQ)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_less(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_LT_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LT_OQ)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_less_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_LE_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LE_OQ)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_EQ_OQ)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_any_not_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ) != 0;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_NEQ_OQ)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_zero() const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, _zero._data, _CMP_EQ_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_zero._data, _data, _CMP_EQ_OQ)) == 0xFF;
#endif
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
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_GT_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GT_OQ)) == 0xFF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_greater_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_GE_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_GE_OQ)) == 0xFF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_less(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_LT_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LT_OQ)) == 0xFF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_less_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_LE_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_LE_OQ)) == 0xFF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_EQ_OQ)) == 0xFF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool EightFloats::
is_all_not_equal(const EightFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ) == 0xFF;
#else
  return _mm256_movemask_ps(_mm256_cmp_ps(_data, other._data, _CMP_NEQ_OQ)) == 0xFF;
#endif
}

/**
 * Selects from other where the bit in mask is not set, and from this
 * vector where the bit is set.
 *
 * Also see blend_zero().
 */
ALWAYS_INLINE EightFloats EightFloats::
blend(const EightFloats &other, const EightFloatsMask &mask) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_mask_blend_ps(mask._mask, other._data, _data);
#else
  return _mm256_blendv_ps(other._data, _data, mask._mask);
#endif
}

/**
 * Selects 0 where the bit in mask is not set, and the current value where the
 * bit is set.
 *
 * Faster than blend(0.0f, mask) on non-AVX512 hardware.
 */
ALWAYS_INLINE EightFloats EightFloats::
blend_zero(const EightFloatsMask &mask) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm256_mask_blend_ps(mask._mask, _zero._data, _data);
#else
  return _mm256_and_ps(_data, mask._mask);
#endif
}

/**
 * Multiplies m1 with m2 and returns the result added onto this vector.
 *
 * Implemented as fused-multiply-add with AVX.
 */
ALWAYS_INLINE EightFloats EightFloats::
madd(const EightFloats &m1, const EightFloats &m2) const {
  return _mm256_fmadd_ps(m1._data, m2._data, _data);
}

/**
 * Multiplies m1 with m2 and returns the result subtracted from this
 * vector.
 *
 * Implemented as fused-multiply-subtract with AVX.
 */
ALWAYS_INLINE EightFloats EightFloats::
msub(const EightFloats &m1, const EightFloats &m2) const {
  return _mm256_fmsub_ps(m1._data, m2._data, _data);
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

/**
 *
 */
ALWAYS_INLINE void
simd_transpose(EightFloats &a, EightFloats &b, EightFloats &c, EightFloats &d) {
  //_MM_TRANSPOSE8_PS(a._data, b._data, c._data, d._data);
}

/**
 *
 */
ALWAYS_INLINE EightVector3s::
EightVector3s(const EightFloats &x, const EightFloats &y, const EightFloats &z) :
  SIMDVector3<EightFloats, EightVector3s>(x, y, z)
{
}

/**
 *
 */
ALWAYS_INLINE EightVector3s::
EightVector3s(const LVecBase3f *vectors) {
  load(vectors);
}

/**
 *
 */
ALWAYS_INLINE EightVector3s::
EightVector3s(const LVecBase3f &vec) {
  load(vec);
}

/**
 * WARNING: Not fast at all.  Try to have the memory already laid out in
 * SOA format.
 */
ALWAYS_INLINE void EightVector3s::
load(const LVecBase3f *v) {
  FourFloats v0, v1, v2, v3, v4, v5, v6, v7;
  v0.load_unaligned(v[0].get_data());
  v1.load_unaligned(v[1].get_data());
  v2.load_unaligned(v[2].get_data());
  v3.load_unaligned(v[3].get_data());
  v4.load_unaligned(v[4].get_data());
  v5.load_unaligned(v[5].get_data());
  v6.load_unaligned(v[6].get_data());
  v7.load_unaligned(v[7].get_data());
  simd_transpose(v0, v1, v2, v3);
  simd_transpose(v4, v5, v6, v7);
  _v[0] = _mm256_set_m128(*v0, *v4);
  _v[1] = _mm256_set_m128(*v1, *v5);
  _v[2] = _mm256_set_m128(*v2, *v6);
}

/**
 *
 */
ALWAYS_INLINE void EightVector3s::
load(const LVecBase3f &fill) {
  _v[0].load(fill[0]);
  _v[1].load(fill[1]);
  _v[2].load(fill[2]);
}
