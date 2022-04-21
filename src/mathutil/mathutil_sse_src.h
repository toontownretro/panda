/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_sse_src.h
 * @author brian
 * @date 2022-04-13
 */

#include "pandabase.h"

// SSE2 base
#include <emmintrin.h>

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#ifdef __SSE4_1__
#ifndef HAVE_BLEND
#define HAVE_BLEND 1
#endif
#include <smmintrin.h>
#endif

#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

#if defined(__FMA__) || defined(__AVX2__)
#ifdef HAVE_FMA
#define HAVE_FMA 1
#endif
#include <immintrin.h> // Included for 128-bit FMA intrinsics.
#endif

#ifdef HAVE_SLEEF

#include <sleef.h>

#if defined(__AVX2__)
#ifndef SLEEF_AVX2_128
#define SLEEF_AVX2_128 1
#endif

#elif defined(__SSE4_1__) || defined(__SSE4_2__)
#ifndef SLEEF_SSE4
#define SLEEF_SSE4 1
#endif

#endif

#endif // HAVE_SLEEF

#ifdef CPPPARSER
typedef int __m128;
typedef int __m128i;
#endif

typedef __m128 PN_vec4f;
typedef __m128i PN_vec4i;

///////////////////////////////////////////////////////////////////////////////
// Memory load/store operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec4f &
simd_fill(PN_vec4f &vec, float val) {
  vec = _mm_set1_ps(val);
  return vec;
}

ALWAYS_INLINE PN_vec4i &
simd_fill(PN_vec4i &vec, int val) {
  vec = _mm_set1_epi32(val);
  return vec;
}

ALWAYS_INLINE PN_vec4f &
simd_set(PN_vec4f &vec, float a, float b, float c, float d) {
  vec = _mm_set_ps(a, b, c, d);
  return vec;
}

ALWAYS_INLINE PN_vec4i &
simd_set(PN_vec4i &vec, int a, int b, int c, int d) {
  vec = _mm_set_epi32(a, b, c, d);
  return vec;
}

ALWAYS_INLINE PN_vec4f &
simd_load_aligned(PN_vec4f &vec, const float *data) {
  vec = _mm_load_ps(data);
  return vec;
}

ALWAYS_INLINE PN_vec4i &
simd_load_aligned(PN_vec4i &vec, const int *data) {
  //vec = _mm_load_ps((const float *)data);
  //return vec;
}

ALWAYS_INLINE PN_vec4f &
simd_load_unaligned(PN_vec4f &vec, const float *data) {
  vec = _mm_loadu_ps(data);
  return vec;
}

ALWAYS_INLINE PN_vec4i &
simd_load_unaligned4iv(PN_vec4i &vec, const int *data) {
  //vec = _mm_loadu_epi32(data);
  //return vec;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Arithetic.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec4f
simd_add(PN_vec4f a, PN_vec4f b) {
  return _mm_add_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_add(PN_vec4i a, PN_vec4i b) {
  return _mm_add_epi32(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_sub(PN_vec4f a, PN_vec4f b) {
  return _mm_sub_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_sub(PN_vec4i a, PN_vec4i b) {
  return _mm_sub_epi32(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_mul(PN_vec4f a, PN_vec4f b) {
  return _mm_mul_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_mul(PN_vec4i a, PN_vec4i b) {
  return _mm_mul_epi32(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_div(PN_vec4f a, PN_vec4f b) {
  return _mm_div_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_div(PN_vec4i a, PN_vec4i b) {
  return a;
  //return _mm_div_epi32(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_madd(PN_vec4f a, PN_vec4f b, PN_vec4f c) {
#ifdef HAVE_FMA
  return _mm_fmadd_ps(a, b, c);
#else
  return simd_add(simd_mul(a, b), c);
#endif
}

ALWAYS_INLINE PN_vec4i
simd_madd(PN_vec4i a, PN_vec4i b, PN_vec4i c) {
  return simd_add(simd_mul(a, b), c);
}

ALWAYS_INLINE PN_vec4f
simd_msub(PN_vec4f a, PN_vec4f b, PN_vec4f c) {
#ifdef HAVE_FMA
  return _mm_fmsub_ps(a, b, c);
#else
  return simd_sub(c, simd_mul(a, b));
#endif
}

ALWAYS_INLINE PN_vec4i
simd_msub(PN_vec4i a, PN_vec4i b, PN_vec4i c) {
  return simd_sub(c, simd_mul(a, b));
}

ALWAYS_INLINE PN_vec4f
simd_neg(PN_vec4f a) {
  return simd_sub(_mm_setzero_ps(), a);
}

ALWAYS_INLINE PN_vec4i
simd_neg(PN_vec4i a) {
  return simd_sub(_mm_setzero_si128(), a);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Logical operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec4f
simd_and(PN_vec4f a, PN_vec4f b) {
  return _mm_and_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_and(PN_vec4i a, PN_vec4i b) {
  return _mm_and_si128(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_or(PN_vec4f a, PN_vec4f b) {
  return _mm_or_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_or(PN_vec4i a, PN_vec4i b) {
  return _mm_or_si128(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_xor(PN_vec4f a, PN_vec4f b) {
  return _mm_xor_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_xor(PN_vec4i a, PN_vec4i b) {
  return _mm_xor_si128(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_not(PN_vec4f a) {
  PN_vec4f b = { };
  PN_vec4f mask = _mm_cmpeq_ps(b, b);
  return _mm_xor_ps(a, mask);
}

ALWAYS_INLINE PN_vec4i
simd_not(PN_vec4i a) {
  PN_vec4i b = { };
  PN_vec4i mask = _mm_cmpeq_epi32(b, b);
  return _mm_xor_si128(a, mask);
}

ALWAYS_INLINE PN_vec4f
simd_andnot(PN_vec4f a, PN_vec4f b) {
  return _mm_andnot_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_andnot(PN_vec4i a, PN_vec4i b) {
  return _mm_andnot_si128(a, b);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Comparison operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec4f
simd_cmp_greater(PN_vec4f a, PN_vec4f b) {
  return _mm_cmpgt_ps(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_cmp_greater_equal(PN_vec4f a, PN_vec4f b) {
  return _mm_cmpge_ps(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_cmp_less(PN_vec4f a, PN_vec4f b) {
  return _mm_cmplt_ps(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_cmp_less_equal(PN_vec4f a, PN_vec4f b) {
  return _mm_cmple_ps(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_cmp_equal(PN_vec4f a, PN_vec4f b) {
  return _mm_cmpeq_ps(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_cmp_not_equal(PN_vec4f a, PN_vec4f b) {
  return _mm_cmpneq_ps(a, b);
}

///////////////////////////////////////////////////////////////////////////////
// Misc operations.
///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PN_vec4f
simd_min(PN_vec4f a, PN_vec4f b) {
  return _mm_min_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_min(PN_vec4i a, PN_vec4i b) {
  return _mm_min_epi32(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_max(PN_vec4f a, PN_vec4f b) {
  return _mm_max_ps(a, b);
}

ALWAYS_INLINE PN_vec4i
simd_max(PN_vec4i a, PN_vec4i b) {
  return _mm_max_epi32(a, b);
}

ALWAYS_INLINE PN_vec4f
simd_sqrt(PN_vec4f a) {
  return _mm_sqrt_ps(a);
}

ALWAYS_INLINE PN_vec4f
simd_rsqrt(PN_vec4f a) {
  return _mm_rsqrt_ps(a);
}

ALWAYS_INLINE PN_vec4f
simd_recip(PN_vec4f a) {
  return _mm_rcp_ps(a);
}

ALWAYS_INLINE int
simd_test_sign(PN_vec4f a) {
  return _mm_movemask_ps(a);
}

ALWAYS_INLINE bool
simd_is_any_negative(PN_vec4f a) {
  return simd_test_sign(a) != 0;
}

ALWAYS_INLINE bool
simd_is_all_off(PN_vec4f a) {
  return simd_test_sign(a) == 0;
}

ALWAYS_INLINE bool
simd_is_all_on(PN_vec4f a) {
  return simd_test_sign(a) == 0xF;
}

ALWAYS_INLINE bool
simd_is_any_off(PN_vec4f a) {
  return simd_test_sign(a) != 0xF;
}

ALWAYS_INLINE bool
simd_is_any_on(PN_vec4f a) {
  return simd_test_sign(a) != 0;
}

ALWAYS_INLINE bool
simd_is_any_greater(PN_vec4f a, PN_vec4f b) {
  return simd_is_any_on(simd_cmp_greater(a, b));
}

ALWAYS_INLINE bool
simd_is_any_greater_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_any_on(simd_cmp_greater_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_any_less(PN_vec4f a, PN_vec4f b) {
  return simd_is_any_on(simd_cmp_less(a, b));
}

ALWAYS_INLINE bool
simd_is_any_less_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_any_on(simd_cmp_less_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_any_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_any_on(simd_cmp_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_any_not_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_any_on(simd_cmp_not_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_greater(PN_vec4f a, PN_vec4f b) {
  return simd_is_all_on(simd_cmp_greater(a, b));
}

ALWAYS_INLINE bool
simd_is_all_greater_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_all_on(simd_cmp_greater_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_less(PN_vec4f a, PN_vec4f b) {
  return simd_is_all_on(simd_cmp_less(a, b));
}

ALWAYS_INLINE bool
simd_is_all_less_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_all_on(simd_cmp_less_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_all_on(simd_cmp_equal(a, b));
}

ALWAYS_INLINE bool
simd_is_all_not_equal(PN_vec4f a, PN_vec4f b) {
  return simd_is_all_on(simd_cmp_not_equal(a, b));
}

ALWAYS_INLINE PN_vec4f
simd_blend(PN_vec4f mask, PN_vec4f a, PN_vec4f b) {
#ifdef HAVE_BLEND
  return _mm_blendv_ps(a, b, mask);
#else
  return simd_or(simd_andnot(a, mask), simd_and(b, mask));
#endif
}

ALWAYS_INLINE PN_vec4f
simd_blend_zero(PN_vec4f mask, PN_vec4f a) {
  return simd_and(a, mask);
}

ALWAYS_INLINE float *
simd_data(PN_vec4f &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<float *>(&a);
#else
  return a.m128_f32;
#endif
}

ALWAYS_INLINE int *
simd_data(PN_vec4i &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<int *>(&a);
#else
  return a.m128i_i32;
#endif
}

ALWAYS_INLINE const float *
simd_data(const PN_vec4f &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<const float *>(&a);
#else
  return a.m128_f32;
#endif
}

ALWAYS_INLINE const int *
simd_data(const PN_vec4i &a) {
#if defined(__clang__) || defined(__GNUC__)
  return reinterpret_cast<const int *>(&a);
#else
  return a.m128i_i32;
#endif
}

ALWAYS_INLINE float &
simd_col(PN_vec4f &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<float *>(&a)[col]);
#else
  return a.m128_f32[col];
#endif
}

ALWAYS_INLINE int &
simd_col(PN_vec4i &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<int *>(&a)[col]);
#else
  return a.m128i_i32[col];
#endif
}

ALWAYS_INLINE const float &
simd_col(const PN_vec4f &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<const float *>(&a)[col]);
#else
  return a.m128_f32[col];
#endif
}

ALWAYS_INLINE const int &
simd_col(const PN_vec4i &a, int col) {
#if defined(__clang__) || defined(__GNUC__)
  return (reinterpret_cast<const int *>(&a)[col]);
#else
  return a.m128i_i32[col];
#endif
}

ALWAYS_INLINE PN_vec4f
simd_sin(PN_vec4f a) {
#ifdef SLEEF_AVX2_128
  return Sleef_sinf4_u35avx2128(a);
#elif defined(SLEEF_SSE4)
  return Sleef_sinf4_u35sse4(a);
#elif defined(HAVE_SLEEF)
  return Sleef_sinf4_u35sse2(a);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = csin(simd_col(a, 0));
  simd_col(ret, 1) = csin(simd_col(a, 1));
  simd_col(ret, 2) = csin(simd_col(a, 2));
  simd_col(ret, 3) = csin(simd_col(a, 3));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec4f
simd_cos(PN_vec4f a) {
#ifdef SLEEF_AVX2_128
  return Sleef_cosf4_u35avx2128(a);
#elif defined(SLEEF_SSE4)
  return Sleef_cosf4_u35sse4(a);
#elif defined(HAVE_SLEEF)
  return Sleef_cosf4_u35sse2(a);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = ccos(simd_col(a, 0));
  simd_col(ret, 1) = ccos(simd_col(a, 1));
  simd_col(ret, 2) = ccos(simd_col(a, 2));
  simd_col(ret, 3) = ccos(simd_col(a, 3));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec4f
simd_tan(PN_vec4f a) {
#ifdef SLEEF_AVX2_128
  return Sleef_tanf4_u35avx2128(a);
#elif defined(SLEEF_SSE4)
  return Sleef_tanf4_u35sse4(a);
#elif defined(HAVE_SLEEF)
  return Sleef_tanf4_u35sse2(a);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = ctan(simd_col(a, 0));
  simd_col(ret, 1) = ctan(simd_col(a, 1));
  simd_col(ret, 2) = ctan(simd_col(a, 2));
  simd_col(ret, 3) = ctan(simd_col(a, 3));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec4f
simd_asin(PN_vec4f a) {
#ifdef SLEEF_AVX2_128
  return Sleef_asinf4_u35avx2128(a);
#elif defined(SLEEF_SSE4)
  return Sleef_asinf4_u35sse4(a);
#elif defined(HAVE_SLEEF)
  return Sleef_asinf4_u35sse2(a);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = casin(simd_col(a, 0));
  simd_col(ret, 1) = casin(simd_col(a, 1));
  simd_col(ret, 2) = casin(simd_col(a, 2));
  simd_col(ret, 3) = casin(simd_col(a, 3));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec4f
simd_acos(PN_vec4f a) {
#ifdef SLEEF_AVX2_128
  return Sleef_acosf4_u35avx2128(a);
#elif defined(SLEEF_SSE4)
  return Sleef_acosf4_u35sse4(a);
#elif defined(HAVE_SLEEF)
  return Sleef_acosf4_u35sse2(a);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = cacos(simd_col(a, 0));
  simd_col(ret, 1) = cacos(simd_col(a, 1));
  simd_col(ret, 2) = cacos(simd_col(a, 2));
  simd_col(ret, 3) = cacos(simd_col(a, 3));
  return ret;
#endif
}

ALWAYS_INLINE PN_vec4f
simd_atan(PN_vec4f a) {
#ifdef SLEEF_AVX2_128
  return Sleef_atanf4_u35avx2128(a);
#elif defined(SLEEF_SSE4)
  return Sleef_atanf4_u35sse4(a);
#elif defined(HAVE_SLEEF)
  return Sleef_atanf4_u35sse2(a);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = catan(simd_col(a, 0));
  simd_col(ret, 1) = catan(simd_col(a, 1));
  simd_col(ret, 2) = catan(simd_col(a, 2));
  simd_col(ret, 3) = catan(simd_col(a, 3));
  return ret;
#endif
}

ALWAYS_INLINE void
simd_sincos(PN_vec4f a, PN_vec4f &sin, PN_vec4f &cos) {
#ifdef SLEEF_AVX2_128
  Sleef___m128_2 ret = Sleef_sincosf4_u35avx2128(a);
  sin = ret.x;
  cos = ret.y;
#elif defined(SLEEF_SSE4)
  Sleef___m128_2 ret = Sleef_sincos4f_u35sse4(a);
  sin = ret.x;
  cos = ret.y;
#elif defined(HAVE_SLEEF)
  Sleef___m128_2 ret = Sleef_sincosf4_u35sse2(a);
  sin = ret.x;
  cos = ret.y;
#else
  csincos(simd_col(a, 0), &simd_col(sin, 0), &simd_col(cos, 0));
  csincos(simd_col(a, 1), &simd_col(sin, 1), &simd_col(cos, 1));
  csincos(simd_col(a, 2), &simd_col(sin, 2), &simd_col(cos, 2));
  csincos(simd_col(a, 3), &simd_col(sin, 3), &simd_col(cos, 3));
#endif
}

ALWAYS_INLINE PN_vec4f
simd_atan2(PN_vec4f a, PN_vec4f b) {
#ifdef SLEEF_AVX2_128
  return Sleef_atan2f4_u35avx2128(a, b);
#elif defined(SLEEF_SSE4)
  return Sleef_atan2f4_u35sse4(a, b);
#elif defined(HAVE_SLEEF)
  return Sleef_atan2f4_u35sse2(a, b);
#else
  PN_vec4f ret;
  simd_col(ret, 0) = catan2(simd_col(a, 0), simd_col(b, 0));
  simd_col(ret, 1) = catan2(simd_col(a, 1), simd_col(b, 1));
  simd_col(ret, 2) = catan2(simd_col(a, 2), simd_col(b, 2));
  simd_col(ret, 3) = catan2(simd_col(a, 3), simd_col(b, 3));
  return ret;
#endif
}

ALWAYS_INLINE void
simd_transpose(PN_vec4f &a, PN_vec4f &b, PN_vec4f &c, PN_vec4f &d) {
  _MM_TRANSPOSE4_PS(a, b, c, d);
}

///////////////////////////////////////////////////////////////////////////////

EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const PN_vec4f &data);
EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const PN_vec4i &data);

typedef SIMDVector<__m128, float> SSEFloatVector;
typedef SIMDVector3<SSEFloatVector> SSEVector3f;
typedef SIMDQuaternion<SSEFloatVector> SSEQuaternionf;
//typedef SIMDVector<__m128d, double> SSEDoubleVector;
//typedef SIMDVector<__m128i, int> SSEIntVector;

#include "mathutil_sse_src.I"

