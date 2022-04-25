/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_avx_src.h
 * @author brian
 * @date 2022-04-13
 */

#include "pandabase.h"

#if defined(__FMA__) || defined(__AVX2__)
#ifdef HAVE_FMA
#define HAVE_FMA 1
#endif
#endif

#include <immintrin.h>

#ifdef HAVE_SLEEF

#include <sleef-config.h>
#include <sleef.h>

#if defined(__AVX2__)
#ifndef SLEEF_AVX2
#define SLEEF_AVX2 1
#endif

#endif

#endif // HAVE_SLEEF

#ifdef CPPPARSER
typedef int __m256;
typedef int __m256i;
#endif

typedef __m256 PN_vec8f;
typedef __m256i PN_vec8i;

///////////////////////////////////////////////////////////////////////////////
// Memory load/store operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec8f &
simd_fill(PN_vec8f &vec, float val) {
  vec = _mm256_set1_ps(val);
  return vec;
}

ALWAYS_INLINE PN_vec8i &
simd_fill(PN_vec8i &vec, int val) {
  vec = _mm256_set1_epi32(val);
  return vec;
}

ALWAYS_INLINE PN_vec8f &
simd_set(PN_vec8f &vec, float a, float b, float c, float d, float e, float f, float g, float h) {
  vec = _mm256_set_ps(a, b, c, d, e, f, g, h);
  return vec;
}

ALWAYS_INLINE PN_vec8i &
simd_set(PN_vec8i &vec, int a, int b, int c, int d, int e, int f, int g, int h) {
  vec = _mm256_set_epi32(a, b, c, d, e, f, g, h);
  return vec;
}

ALWAYS_INLINE PN_vec8f &
simd_load_aligned(PN_vec8f &vec, const float *data) {
  vec = _mm256_load_ps(data);
  return vec;
}

ALWAYS_INLINE PN_vec8i &
simd_load_aligned(PN_vec8i &vec, const int *data) {
  //vec = _mm256_load_ps((const float *)data);
  return vec;
}

ALWAYS_INLINE PN_vec8f &
simd_load_unaligned(PN_vec8f &vec, const float *data) {
  vec = _mm256_loadu_ps(data);
  return vec;
}

ALWAYS_INLINE PN_vec8i &
simd_load_unaligned4iv(PN_vec8i &vec, const int *data) {
  //vec = _mm256_loadu_epi32(data);
  return vec;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Arithetic.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec8f
simd_add(PN_vec8f a, PN_vec8f b) {
  return _mm256_add_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_add(PN_vec8i a, PN_vec8i b) {
  return _mm256_add_epi32(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_sub(PN_vec8f a, PN_vec8f b) {
  return _mm256_sub_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_sub(PN_vec8i a, PN_vec8i b) {
  return _mm256_sub_epi32(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_mul(PN_vec8f a, PN_vec8f b) {
  return _mm256_mul_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_mul(PN_vec8i a, PN_vec8i b) {
  return _mm256_mul_epi32(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_div(PN_vec8f a, PN_vec8f b) {
  return _mm256_div_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_div(PN_vec8i a, PN_vec8i b) {
  return a;
  //return _mm256_div_epi32(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_madd(PN_vec8f a, PN_vec8f b, PN_vec8f c) {
#ifdef HAVE_FMA
  return _mm256_fmadd_ps(a, b, c);
#else
  return simd_add(simd_mul(a, b), c);
#endif
}

ALWAYS_INLINE PN_vec8i
simd_madd(PN_vec8i a, PN_vec8i b, PN_vec8i c) {
  return simd_add(simd_mul(a, b), c);
}

ALWAYS_INLINE PN_vec8f
simd_msub(PN_vec8f a, PN_vec8f b, PN_vec8f c) {
#ifdef HAVE_FMA
  return _mm256_fmsub_ps(a, b, c);
#else
  return simd_sub(c, simd_mul(a, b));
#endif
}

ALWAYS_INLINE PN_vec8i
simd_msub(PN_vec8i a, PN_vec8i b, PN_vec8i c) {
  return simd_sub(c, simd_mul(a, b));
}

ALWAYS_INLINE PN_vec8f
simd_neg(PN_vec8f a) {
  return simd_sub(_mm256_setzero_ps(), a);
}

ALWAYS_INLINE PN_vec8i
simd_neg(PN_vec8i a) {
  return simd_sub(_mm256_setzero_si256(), a);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Logical operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec8f
simd_and(PN_vec8f a, PN_vec8f b) {
  return _mm256_and_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_and(PN_vec8i a, PN_vec8i b) {
  return _mm256_and_si256(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_or(PN_vec8f a, PN_vec8f b) {
  return _mm256_or_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_or(PN_vec8i a, PN_vec8i b) {
  return _mm256_or_si256(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_xor(PN_vec8f a, PN_vec8f b) {
  return _mm256_xor_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_xor(PN_vec8i a, PN_vec8i b) {
  return _mm256_xor_si256(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_not(PN_vec8f a) {
  PN_vec8f b = { };
  PN_vec8f mask = _mm256_cmp_ps(b, b, _CMP_EQ_OQ);
  return _mm256_xor_ps(a, mask);
}

ALWAYS_INLINE PN_vec8i
simd_not(PN_vec8i a) {
  PN_vec8i b = { };
  PN_vec8i mask = _mm256_cmpeq_epi32(b, b);
  return _mm256_xor_si256(a, mask);
}

ALWAYS_INLINE PN_vec8f
simd_andnot(PN_vec8f a, PN_vec8f b) {
  return _mm256_andnot_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_andnot(PN_vec8i a, PN_vec8i b) {
  return _mm256_andnot_si256(a, b);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Comparison operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec8f
simd_cmp_greater(PN_vec8f a, PN_vec8f b) {
  return _mm256_cmp_ps(a, b, _CMP_GT_OQ);
}

ALWAYS_INLINE PN_vec8f
simd_cmp_greater_equal(PN_vec8f a, PN_vec8f b) {
  return _mm256_cmp_ps(a, b, _CMP_GE_OQ);
}

ALWAYS_INLINE PN_vec8f
simd_cmp_less(PN_vec8f a, PN_vec8f b) {
  return _mm256_cmp_ps(a, b, _CMP_LT_OQ);
}

ALWAYS_INLINE PN_vec8f
simd_cmp_less_equal(PN_vec8f a, PN_vec8f b) {
  return _mm256_cmp_ps(a, b, _CMP_LE_OQ);
}

ALWAYS_INLINE PN_vec8f
simd_cmp_equal(PN_vec8f a, PN_vec8f b) {
  return _mm256_cmp_ps(a, b, _CMP_EQ_OQ);
}

ALWAYS_INLINE PN_vec8f
simd_cmp_not_equal(PN_vec8f a, PN_vec8f b) {
  return _mm256_cmp_ps(a, b, _CMP_NEQ_OQ);
}

///////////////////////////////////////////////////////////////////////////////
// Misc operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec8f
simd_min(PN_vec8f a, PN_vec8f b) {
  return _mm256_min_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_min(PN_vec8i a, PN_vec8i b) {
  return _mm256_min_epi32(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_max(PN_vec8f a, PN_vec8f b) {
  return _mm256_max_ps(a, b);
}

ALWAYS_INLINE PN_vec8i
simd_max(PN_vec8i a, PN_vec8i b) {
  return _mm256_max_epi32(a, b);
}

ALWAYS_INLINE PN_vec8f
simd_sqrt(PN_vec8f a) {
  return _mm256_sqrt_ps(a);
}

ALWAYS_INLINE PN_vec8f
simd_rsqrt(PN_vec8f a) {
  return _mm256_rsqrt_ps(a);
}

ALWAYS_INLINE PN_vec8f
simd_recip(PN_vec8f a) {
  return _mm256_rcp_ps(a);
}

ALWAYS_INLINE int
simd_test_sign(PN_vec8f a) {
  return _mm256_movemask_ps(a);
}

ALWAYS_INLINE bool
simd_is_any_negative(PN_vec8f a) {
  return simd_test_sign(a) != 0;
}

ALWAYS_INLINE bool
simd_is_all_off(PN_vec8f a) {
  return simd_test_sign(a) == 0;
}

ALWAYS_INLINE bool
simd_is_all_on(PN_vec8f a) {
  return simd_test_sign(a) == 0xFF;
}

ALWAYS_INLINE bool
simd_is_any_off(PN_vec8f a) {
  return simd_test_sign(a) != 0xFF;
}

ALWAYS_INLINE bool
simd_is_any_on(PN_vec8f a) {
  return simd_test_sign(a) != 0;
}

ALWAYS_INLINE bool
simd_is_any_greater(PN_vec8f a, PN_vec8f b) {
  return simd_is_any_on(simd_cmp_greater(a, b));
}

ALWAYS_INLINE bool
simd_is_any_greater_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_any_on(simd_cmp_greater_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_any_less(PN_vec8f a, PN_vec8f b) {
  return simd_is_any_on(simd_cmp_less(a, b));
}

ALWAYS_INLINE bool
simd_is_any_less_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_any_on(simd_cmp_less_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_any_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_any_on(simd_cmp_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_any_not_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_any_on(simd_cmp_not_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_greater(PN_vec8f a, PN_vec8f b) {
  return simd_is_all_on(simd_cmp_greater(a, b));
}

ALWAYS_INLINE bool
simd_is_all_greater_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_all_on(simd_cmp_greater_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_less(PN_vec8f a, PN_vec8f b) {
  return simd_is_all_on(simd_cmp_less(a, b));
}

ALWAYS_INLINE bool
simd_is_all_less_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_all_on(simd_cmp_less_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_all_on(simd_cmp_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_not_equal(PN_vec8f a, PN_vec8f b) {
  return simd_is_all_on(simd_cmp_not_equal(a, b));
}

ALWAYS_INLINE PN_vec8f
simd_blend(PN_vec8f mask, PN_vec8f a, PN_vec8f b) {
  return _mm256_blendv_ps(a, b, mask);
}

ALWAYS_INLINE PN_vec8f
simd_blend_zero(PN_vec8f mask, PN_vec8f a) {
  return simd_and(a, mask);
}

ALWAYS_INLINE float *
simd_data(PN_vec8f &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<float *>(&a);
#else
  return a.m256_f32;
#endif
}

ALWAYS_INLINE int *
simd_data(PN_vec8i &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<int *>(&a);
#else
  return a.m256i_i32;
#endif
}

ALWAYS_INLINE const float *
simd_data(const PN_vec8f &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<const float *>(&a);
#else
  return a.m256_f32;
#endif
}

ALWAYS_INLINE const int *
simd_data(const PN_vec8i &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<const int *>(&a);
#else
  return a.m256i_i32;
#endif
}

ALWAYS_INLINE float &
simd_col(PN_vec8f &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<float *>(&a)[col]);
#else
  return a.m256_f32[col];
#endif
}

ALWAYS_INLINE int &
simd_col(PN_vec8i &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<int *>(&a)[col]);
#else
  return a.m256i_i32[col];
#endif
}

ALWAYS_INLINE const float &
simd_col(const PN_vec8f &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<const float *>(&a)[col]);
#else
  return a.m256_f32[col];
#endif
}

ALWAYS_INLINE const int &
simd_col(const PN_vec8i &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<const int *>(&a)[col]);
#else
  return a.m256i_i32[col];
#endif
}

ALWAYS_INLINE PN_vec8f
simd_sin(PN_vec8f a) {
#ifdef SLEEF_AVX2
  return Sleef_sinf8_u35avx2(a);
#elif defined(HAVE_SLEEF)
  return Sleef_sinf8_u35avx(a);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = csin(simd_col(a, 0));
  simd_col(ret, 1) = csin(simd_col(a, 1));
  simd_col(ret, 2) = csin(simd_col(a, 2));
  simd_col(ret, 3) = csin(simd_col(a, 3));
  simd_col(ret, 4) = csin(simd_col(a, 4));
  simd_col(ret, 5) = csin(simd_col(a, 5));
  simd_col(ret, 6) = csin(simd_col(a, 6));
  simd_col(ret, 7) = csin(simd_col(a, 7));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec8f
simd_cos(PN_vec8f a) {
#ifdef SLEEF_AVX2
  return Sleef_cosf8_u35avx2(a);
#elif defined(HAVE_SLEEF)
  return Sleef_cosf8_u35avx(a);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = ccos(simd_col(a, 0));
  simd_col(ret, 1) = ccos(simd_col(a, 1));
  simd_col(ret, 2) = ccos(simd_col(a, 2));
  simd_col(ret, 3) = ccos(simd_col(a, 3));
  simd_col(ret, 4) = ccos(simd_col(a, 4));
  simd_col(ret, 5) = ccos(simd_col(a, 5));
  simd_col(ret, 6) = ccos(simd_col(a, 6));
  simd_col(ret, 7) = ccos(simd_col(a, 7));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec8f
simd_tan(PN_vec8f a) {
#ifdef SLEEF_AVX2
  return Sleef_tanf8_u35avx2(a);
#elif defined(HAVE_SLEEF)
  return Sleef_tanf8_u35avx(a);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = ctan(simd_col(a, 0));
  simd_col(ret, 1) = ctan(simd_col(a, 1));
  simd_col(ret, 2) = ctan(simd_col(a, 2));
  simd_col(ret, 3) = ctan(simd_col(a, 3));
  simd_col(ret, 4) = ctan(simd_col(a, 4));
  simd_col(ret, 5) = ctan(simd_col(a, 5));
  simd_col(ret, 6) = ctan(simd_col(a, 6));
  simd_col(ret, 7) = ctan(simd_col(a, 7));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec8f
simd_asin(PN_vec8f a) {
#ifdef SLEEF_AVX2
  return Sleef_asinf8_u35avx2(a);
#elif defined(HAVE_SLEEF)
  return Sleef_asinf8_u35avx(a);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = casin(simd_col(a, 0));
  simd_col(ret, 1) = casin(simd_col(a, 1));
  simd_col(ret, 2) = casin(simd_col(a, 2));
  simd_col(ret, 3) = casin(simd_col(a, 3));
  simd_col(ret, 4) = casin(simd_col(a, 4));
  simd_col(ret, 5) = casin(simd_col(a, 5));
  simd_col(ret, 6) = casin(simd_col(a, 6));
  simd_col(ret, 7) = casin(simd_col(a, 7));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec8f
simd_acos(PN_vec8f a) {
#ifdef SLEEF_AVX2
  return Sleef_acosf8_u35avx2(a);
#elif defined(HAVE_SLEEF)
  return Sleef_acosf8_u35avx(a);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = cacos(simd_col(a, 0));
  simd_col(ret, 1) = cacos(simd_col(a, 1));
  simd_col(ret, 2) = cacos(simd_col(a, 2));
  simd_col(ret, 3) = cacos(simd_col(a, 3));
  simd_col(ret, 4) = cacos(simd_col(a, 4));
  simd_col(ret, 5) = cacos(simd_col(a, 5));
  simd_col(ret, 6) = cacos(simd_col(a, 6));
  simd_col(ret, 7) = cacos(simd_col(a, 7));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec8f
simd_atan(PN_vec8f a) {
#ifdef SLEEF_AVX2
  return Sleef_atanf8_u35avx2(a);
#elif defined(HAVE_SLEEF)
  return Sleef_atanf8_u35avx(a);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = catan(simd_col(a, 0));
  simd_col(ret, 1) = catan(simd_col(a, 1));
  simd_col(ret, 2) = catan(simd_col(a, 2));
  simd_col(ret, 3) = catan(simd_col(a, 3));
  simd_col(ret, 4) = catan(simd_col(a, 4));
  simd_col(ret, 5) = catan(simd_col(a, 5));
  simd_col(ret, 6) = catan(simd_col(a, 6));
  simd_col(ret, 7) = catan(simd_col(a, 7));
  return ret;
#endif
}

ALWAYS_INLINE void
simd_sincos(PN_vec8f a, PN_vec8f &sin, PN_vec8f &cos) {
#ifdef SLEEF_AVX2
  Sleef___m256_2 ret = Sleef_sincosf8_u35avx2(a);
  sin = ret.x;
  cos = ret.y;
#elif defined(HAVE_SLEEF)
  Sleef___m256_2 ret = Sleef_sincosf8_u35avx(a);
  sin = ret.x;
  cos = ret.y;
#else
  csincos(simd_col(a, 0), &simd_col(sin, 0), &simd_col(cos, 0));
  csincos(simd_col(a, 1), &simd_col(sin, 1), &simd_col(cos, 1));
  csincos(simd_col(a, 2), &simd_col(sin, 2), &simd_col(cos, 2));
  csincos(simd_col(a, 3), &simd_col(sin, 3), &simd_col(cos, 3));
  csincos(simd_col(a, 4), &simd_col(sin, 4), &simd_col(cos, 4));
  csincos(simd_col(a, 5), &simd_col(sin, 5), &simd_col(cos, 5));
  csincos(simd_col(a, 6), &simd_col(sin, 6), &simd_col(cos, 6));
  csincos(simd_col(a, 7), &simd_col(sin, 7), &simd_col(cos, 7));
#endif
}

ALWAYS_INLINE PN_vec8f
simd_atan2(PN_vec8f a, PN_vec8f b) {
#ifdef SLEEF_AVX2
  return Sleef_atan2f8_u35avx2(a, b);
#elif defined(HAVE_SLEEF)
  return Sleef_atan2f8_u35avx(a, b);
#else
  PN_vec8f ret;
  simd_col(ret, 0) = catan2(simd_col(a, 0), simd_col(b, 0));
  simd_col(ret, 1) = catan2(simd_col(a, 1), simd_col(b, 1));
  simd_col(ret, 2) = catan2(simd_col(a, 2), simd_col(b, 2));
  simd_col(ret, 3) = catan2(simd_col(a, 3), simd_col(b, 3));
  simd_col(ret, 4) = catan2(simd_col(a, 4), simd_col(b, 4));
  simd_col(ret, 5) = catan2(simd_col(a, 5), simd_col(b, 5));
  simd_col(ret, 6) = catan2(simd_col(a, 6), simd_col(b, 6));
  simd_col(ret, 7) = catan2(simd_col(a, 7), simd_col(b, 7));
  return ret;
#endif
}

///////////////////////////////////////////////////////////////////////////////

EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const PN_vec8f &data);
EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const PN_vec8i &data);

typedef SIMDVector<__m256, float> AVXFloatVector;
typedef SIMDVector3<AVXFloatVector> AVXVector3f;
typedef SIMDQuaternion<AVXFloatVector> AVXQuaternionf;

#include "mathutil_avx_src.I"
