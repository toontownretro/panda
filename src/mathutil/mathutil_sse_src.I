/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_sse_src.I
 * @author brian
 * @date 2022-04-13
 */

/**
 *
 */
ALWAYS_INLINE FourFloatsMask::
FourFloatsMask(PN_vec4f_mask &&mask) :
  _mask(std::move(mask))
{
}

/**
 *
 */
ALWAYS_INLINE void FourFloatsMask::
operator = (PN_vec4f_mask &&mask) {
  _mask = std::move(mask);
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloatsMask::
operator & (const FourFloatsMask &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mask & other._mask;
#else
  return _mm_and_ps(_mask, other._mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloatsMask::
operator | (const FourFloatsMask &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mask | other._mask;
#else
  return _mm_or_ps(_mask, other._mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloatsMask::
operator ^ (const FourFloatsMask &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mask ^ other._mask;
#else
  return _mm_xor_ps(_mask, other._mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloatsMask::
operator ~ () const {
#ifdef HAVE_MASK_REGISTERS
  return ~_mask;
#else
  return _mm_xor_ps(FourFloats::_negative_one._data, _mask);
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloatsMask::
is_all_on() const {
#ifdef HAVE_MASK_REGISTERS
  return _mask == 0xF;
#else
  return _mm_movemask_ps(_mask) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloatsMask::
is_all_off() const {
#ifdef HAVE_MASK_REGISTERS
  return _mask == 0;
#else
  return _mm_movemask_ps(_mask) == 0;
#endif
}

/**
 * Fills all components of the float vector/SIMD register with a single
 * float value.
 */
ALWAYS_INLINE FourFloats::
FourFloats(float fill) {
  _data = _mm_set_ps1(fill);
}

/**
 * Fills the float vector/SIMD register with the given separate float
 * values for each component.
 */
ALWAYS_INLINE FourFloats::
FourFloats(float a, float b, float c, float d) {
  _data = _mm_set_ps(a, b, c, d);
}

/**
 *
 */
ALWAYS_INLINE FourFloats::
FourFloats(const float *data, bool aligned) {
  if (aligned) {
    _data = _mm_load_ps(data);
  } else {
    _data = _mm_loadu_ps(data);
  }
}

/**
 *
 */
ALWAYS_INLINE FourFloats::
FourFloats(const PN_vec4f &data) :
  _data(data)
{
}

/**
 *
 */
ALWAYS_INLINE FourFloats::
FourFloats(PN_vec4f &&data) :
  _data(std::move(data))
{
}

/**
 *
 */
ALWAYS_INLINE FourFloats::
FourFloats(const FourFloats &other) :
  _data(other._data)
{
}

/**
 *
 */
ALWAYS_INLINE FourFloats::
FourFloats(FourFloats &&other) :
  _data(std::move(other._data))
{
}

/**
 * Assuming that the vector is not already in an SIMD register, loads the
 * current values of the vector into an SIMD register, and stores the new
 * vector on this object.
 */
ALWAYS_INLINE void FourFloats::
load() {
#if defined(__clang__) || defined(__GNUC__)
  _data = _mm_load_ps(reinterpret_cast<const float *>(&_data));
#else
  _data = _mm_load_ps(_data.m128_f32);
#endif
}

/**
 * Loads a single value into all components of an SIMD register,
 * and stores the new vector on this object.
 */
ALWAYS_INLINE void FourFloats::
load(float fill) {
  _data = _mm_load1_ps(&fill);
}

/**
 * Loads four separate float values into an SIMD register, and stores
 * the new vector on this object.
 */
ALWAYS_INLINE void FourFloats::
load(float a, float b, float c, float d) {
  _data = _mm_set_ps(a, b, c, d);
}

/**
 * Loads four floats from the given contiguous array of floats into an
 * SIMD register, and stores the new vector on this object.
 *
 * This version assumes the float array is aligned to 16-byte boundaries.
 * Use load_unaligned() for unaligned loads.
 */
ALWAYS_INLINE void FourFloats::
load(const float *data) {
  _data = _mm_load_ps(data);
}

/**
 * Loads four floats from the given contiguous array of floats into an
 * SIMD register, and stores the new vector on this object.
 *
 * This version assumes the float array is not aligned to 16-byte boundaries.
 * Use load() for aligned loads.
 */
ALWAYS_INLINE void FourFloats::
load_unaligned(const float *data) {
  _data = _mm_loadu_ps(data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator * (const FourFloats &other) const {
  return _mm_mul_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator / (const FourFloats &other) const {
  return _mm_div_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator - (const FourFloats &other) const {
  return _mm_sub_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator + (const FourFloats &other) const {
  return _mm_add_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator & (const FourFloats &other) const {
  return _mm_and_ps(_data, other._data);
}

/**
 * Shorthand for blend_zero().
 *
 * Does a masked blend with 0 on AVX-512, an actual AND otherwise.
 */
ALWAYS_INLINE FourFloats FourFloats::
operator & (const FourFloatsMask &mask) const {
  return blend_zero(mask);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator | (const FourFloats &other) const {
  return _mm_or_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator ^ (const FourFloats &other) const {
  return _mm_xor_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
operator - () const {
  return _mm_sub_ps(_mm_setzero_ps(), _data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator *= (const FourFloats &other) {
  _data = _mm_mul_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator /= (const FourFloats &other) {
  _data = _mm_div_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator -= (const FourFloats &other) {
  _data = _mm_sub_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator += (const FourFloats &other) {
  _data = _mm_add_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator &= (const FourFloats &other) {
  _data = _mm_and_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator |= (const FourFloats &other) {
  _data = _mm_or_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator ^= (const FourFloats &other) {
  _data = _mm_xor_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE const PN_vec4f &FourFloats::
operator * () const {
  return _data;
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator = (const FourFloats &other) {
  _data = other._data;
}

/**
 *
 */
ALWAYS_INLINE void FourFloats::
operator = (FourFloats &&other) {
  _data = std::move(other._data);
}

/**
 *
 */
ALWAYS_INLINE float FourFloats::
operator [] (int n) const {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<const float *>(&_data))[n];
#else
  return _data.m128_f32[n];
#endif
}

ALWAYS_INLINE float &FourFloats::
operator [] (int n) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<float *>(&_data))[n];
#else
  return _data.m128_f32[n];
#endif
}

/**
 *
 */
ALWAYS_INLINE float *FourFloats::
modify_data() {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<float *>(&_data);
#else
  return _data.m128_f32;
#endif
}

/**
 *
 */
ALWAYS_INLINE const float *FourFloats::
get_data() const {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<const float *>(&_data);
#else
  return _data.m128_f32;
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloats::
operator > (const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_GT_OQ);
#else
  return _mm_cmpgt_ps(_data, other._data);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloats::
operator >= (const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_GE_OQ);
#else
  return _mm_cmpge_ps(_data, other._data);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloats::
operator < (const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_LT_OQ);
#else
  return _mm_cmplt_ps(_data, other._data);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloats::
operator <= (const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_LE_OQ);
#else
  return _mm_cmple_ps(_data, other._data);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloats::
operator == (const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ);
#else
  return _mm_cmpeq_ps(_data, other._data);
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloatsMask FourFloats::
operator != (const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ);
#else
  return _mm_cmpneq_ps(_data, other._data);
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_zero() const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, _zero._data, _CMP_EQ_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmpeq_ps(_data, _zero._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_negative() const {
  return _mm_movemask_ps(_data) != 0;
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_greater(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_GT_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmpgt_ps(_data, other._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_greater_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_GE_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmpge_ps(_data, other._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_less(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_LT_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmplt_ps(_data, other._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_less_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_LE_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmple_ps(_data, other._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmpeq_ps(_data, other._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_any_not_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ) != 0;
#else
  return _mm_movemask_ps(_mm_cmpneq_ps(_data, other._data)) != 0;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_zero() const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, _zero._data, _CMP_EQ_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmpeq_ps(_data, _zero._data)) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_negative() const {
  return _mm_movemask_ps(_data) == 0xF;
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_greater(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_GT_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmpgt_ps(_data, other._data)) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_greater_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_GE_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmpge_ps(_data, other._data)) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_less(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_LT_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmplt_ps(_data, other._data)) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_less_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_LE_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmple_ps(_data, other._data)) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_EQ_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmpeq_ps(_data, other._data)) == 0xF;
#endif
}

/**
 *
 */
ALWAYS_INLINE bool FourFloats::
is_all_not_equal(const FourFloats &other) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_cmp_ps_mask(_data, other._data, _CMP_NEQ_OQ) == 0xF;
#else
  return _mm_movemask_ps(_mm_cmpneq_ps(_data, other._data)) == 0xF;
#endif
}

/**
 * Selects from other where the bit in mask is not set, and from this
 * vector where the bit is set.
 *
 * Also see blend_zero().
 */
ALWAYS_INLINE FourFloats FourFloats::
blend(const FourFloats &other, const FourFloatsMask &mask) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_mask_blend_ps(mask._mask, other._data, _data);
#elif defined(__SSE4_1__)
  return _mm_blendv_ps(other._data, _data, mask._mask);
#else
  // No blending support on older than SSE4.1.
  return _mm_add_ps(_mm_and_ps(_data, mask._mask), _mm_andnot_ps(mask._mask, other._data));
#endif
}

/**
 * Selects 0 where the bit in mask is not set, and the current value where the
 * bit is set.
 *
 * Faster than blend(0.0f, mask) on non-AVX512 hardware.
 */
ALWAYS_INLINE FourFloats FourFloats::
blend_zero(const FourFloatsMask &mask) const {
#ifdef HAVE_MASK_REGISTERS
  return _mm_mask_blend_ps(mask._mask, _zero._data, _data);
#else
  return _mm_and_ps(_data, mask._mask);
#endif
}

/**
 * Multiplies m1 with m2 and returns the result added onto this vector.
 *
 * Implemented as fused-multiply-add with AVX.
 */
ALWAYS_INLINE FourFloats FourFloats::
madd(const FourFloats &m1, const FourFloats &m2) const {
#ifdef __AVX__
  return _mm_fmadd_ps(m1._data, m2._data, _data);
#else
  return _mm_add_ps(_mm_mul_ps(m1._data, m2._data), _data);
#endif
}

/**
 * Multiplies m1 with m2 and returns the result subtracted from this
 * vector.
 *
 * Implemented as fused-multiply-subtract with AVX.
 */
ALWAYS_INLINE FourFloats FourFloats::
msub(const FourFloats &m1, const FourFloats &m2) const {
#ifdef __AVX__
  return _mm_fmsub_ps(m1._data, m2._data, _data);
#else
  return _mm_sub_ps(_data, _mm_mul_ps(m1._data, m2._data));
#endif
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
min(const FourFloats &other) const {
  return _mm_min_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
max(const FourFloats &other) const {
  return _mm_max_ps(_data, other._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
sqrt() const {
  return _mm_sqrt_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
rsqrt() const {
  return _mm_rsqrt_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats FourFloats::
recip() const {
  return _mm_rcp_ps(_data);
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
zero() {
  return _zero;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
one() {
  return _one;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
negative_one() {
  return _negative_one;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
two() {
  return _two;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
three() {
  return _three;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
four() {
  return _four;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
point_five() {
  return _point_five;
}

/**
 *
 */
ALWAYS_INLINE const FourFloats &FourFloats::
flt_epsilon() {
  return _flt_epsilon;
}

/**
 *
 */
ALWAYS_INLINE FourFloats
simd_min(const FourFloats &a, const FourFloats &b) {
  return _mm_min_ps(a._data, b._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats
simd_max(const FourFloats &a, const FourFloats &b) {
  return _mm_max_ps(a._data, b._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats
simd_sqrt(const FourFloats &val) {
  return _mm_sqrt_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats
simd_rsqrt(const FourFloats &val) {
  return _mm_rsqrt_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE FourFloats
simd_recip(const FourFloats &val) {
  return _mm_rcp_ps(val._data);
}

/**
 *
 */
ALWAYS_INLINE void
simd_transpose(FourFloats &a, FourFloats &b, FourFloats &c, FourFloats &d) {
  _MM_TRANSPOSE4_PS(a._data, b._data, c._data, d._data);
}

/**
 *
 */
ALWAYS_INLINE FourVector3s::
FourVector3s(const FourFloats &x, const FourFloats &y, const FourFloats &z) :
  SIMDVector3<FourFloats, FourVector3s>(x, y, z)
{
}

/**
 *
 */
ALWAYS_INLINE FourVector3s::
FourVector3s(const LVecBase3f *vectors) {
  load(vectors);
}

/**
 *
 */
ALWAYS_INLINE FourVector3s::
FourVector3s(const LVecBase3f &a, const LVecBase3f &b, const LVecBase3f &c, const LVecBase3f &d) {
  load(a, b, c, d);
}
/**
 *
 */
ALWAYS_INLINE FourVector3s::
FourVector3s(const LVecBase3f &vec) {
  load(vec);
}

/**
 *
 */
ALWAYS_INLINE void FourVector3s::
load(const LVecBase3f *vectors) {
  FourFloats v3;
  _v[0].load_unaligned(vectors[0].get_data());
  _v[1].load_unaligned(vectors[1].get_data());
  _v[2].load_unaligned(vectors[2].get_data());
  v3.load_unaligned(vectors[3].get_data());
  // x y z ?
  // x y z ?
  // x y z ?
  // x y z ?
  // TO
  // x x x x
  // y y y y
  // z z z z
  simd_transpose(_v[0], _v[1], _v[2], v3);
}
/**
 *
 */
ALWAYS_INLINE void FourVector3s::
load(const LVecBase3f &a, const LVecBase3f &b, const LVecBase3f &c, const LVecBase3f &d) {
  FourFloats v3;
  _v[0].load_unaligned(a.get_data());
  _v[1].load_unaligned(b.get_data());
  _v[2].load_unaligned(c.get_data());
  v3.load_unaligned(d.get_data());
  // x y z ?
  // x y z ?
  // x y z ?
  // x y z ?
  // TO
  // x x x x
  // y y y y
  // z z z z
  simd_transpose(_v[0], _v[1], _v[2], v3);
}

/**
 *
 */
ALWAYS_INLINE void FourVector3s::
load(const LVecBase3f &fill) {
  _v[0].load(fill[0]);
  _v[1].load(fill[1]);
  _v[2].load(fill[2]);
}
