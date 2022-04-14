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

// SSE2 base
#include <emmintrin.h>

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

#ifdef CPPPARSER
typedef int __m128;
typedef int __m128d;
typedef int __m128i;
typedef int __mmask8;
#endif

#if defined(__AVX512F__) && defined(__AVX512VL__)
// With AVX-512, we can compare directly into a mask.
#include <zmmintrin.h>
#ifndef HAVE_MASK_REGISTERS
#define HAVE_MASK_REGISTERS 1
#endif
typedef __mmask8 PN_vec4f_mask;
typedef __mmask8 PN_vec2d_mask;
typedef __mmask8 PN_vec4i_mask;
#else
typedef __m128 PN_vec4f_mask;
typedef __m128d PN_vec2d_mask;
typedef __m128i PN_vec4i_mask;
#endif

typedef __m128 PN_vec4f;
typedef __m128d PN_vec2d;
typedef __m128i PN_vec4i;

/**
 * Returned from comparisons between two FourFloats.
 *
 * With AVX-512, this is an actual mask in a mask register with a bit set
 * for each column that evaluated true.
 *
 * Without AVX-512, this is a vector storing ~0 for true, and 0 for false,
 * in each column.
 */
class FourFloatsMask {
PUBLISHED:
  FourFloatsMask() = delete;
  FourFloatsMask(const PN_vec4f_mask &mask) = delete;
  void operator = (const PN_vec4f_mask &mask) = delete;

  ALWAYS_INLINE FourFloatsMask(PN_vec4f_mask &&mask);
  void operator = (PN_vec4f_mask &&mask);

  ALWAYS_INLINE FourFloatsMask operator & (const FourFloatsMask &other) const;
  ALWAYS_INLINE FourFloatsMask operator | (const FourFloatsMask &other) const;
  ALWAYS_INLINE FourFloatsMask operator ^ (const FourFloatsMask &other) const;
  ALWAYS_INLINE FourFloatsMask operator ~ () const;

  ALWAYS_INLINE bool is_all_on() const;
  ALWAYS_INLINE bool is_all_off() const;

public:
  PN_vec4f_mask _mask;
};

/**
 * Four 32-bit floats occupying a single M128 SIMD register.
 *
 * SSE
 */
class EXPCL_PANDA_MATHUTIL ALIGN_16BYTE FourFloats {
PUBLISHED:
  static const int width = 4;

  typedef FourFloatsMask MaskType;

  ALWAYS_INLINE FourFloats() = default;
  ALWAYS_INLINE FourFloats(float fill);
  ALWAYS_INLINE FourFloats(float a, float b, float c, float d);
  ALWAYS_INLINE FourFloats(const float *data, bool aligned);
  ALWAYS_INLINE FourFloats(const PN_vec4f &data);
  ALWAYS_INLINE FourFloats(PN_vec4f &&data);
  ALWAYS_INLINE FourFloats(const FourFloats &other);
  ALWAYS_INLINE FourFloats(FourFloats &&other);

  ALWAYS_INLINE void load();
  ALWAYS_INLINE void load(float fill);
  ALWAYS_INLINE void load(float a, float b, float c, float d);
  ALWAYS_INLINE void load(const float *data);
  ALWAYS_INLINE void load_unaligned(const float *data);

  ALWAYS_INLINE FourFloats operator * (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator / (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator - (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator + (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator & (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator & (const FourFloatsMask &mask) const;
  ALWAYS_INLINE FourFloats operator | (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator ^ (const FourFloats &other) const;
  ALWAYS_INLINE FourFloats operator - () const;

  ALWAYS_INLINE FourFloatsMask operator > (const FourFloats &other) const;
  ALWAYS_INLINE FourFloatsMask operator >= (const FourFloats &other) const;
  ALWAYS_INLINE FourFloatsMask operator < (const FourFloats &other) const;
  ALWAYS_INLINE FourFloatsMask operator <= (const FourFloats &other) const;
  ALWAYS_INLINE FourFloatsMask operator == (const FourFloats &other) const;
  ALWAYS_INLINE FourFloatsMask operator != (const FourFloats &other) const;

  ALWAYS_INLINE void operator *= (const FourFloats &other);
  ALWAYS_INLINE void operator /= (const FourFloats &other);
  ALWAYS_INLINE void operator -= (const FourFloats &other);
  ALWAYS_INLINE void operator += (const FourFloats &other);
  ALWAYS_INLINE void operator &= (const FourFloats &other);
  ALWAYS_INLINE void operator |= (const FourFloats &other);
  ALWAYS_INLINE void operator ^= (const FourFloats &other);

  ALWAYS_INLINE const PN_vec4f &operator * () const;

  ALWAYS_INLINE void operator = (const FourFloats &other);
  ALWAYS_INLINE void operator = (FourFloats &&other);

  ALWAYS_INLINE float operator [] (int n) const;
  ALWAYS_INLINE float &operator [] (int n);
  ALWAYS_INLINE float *modify_data();
  ALWAYS_INLINE const float *get_data() const;

  ALWAYS_INLINE bool is_any_zero() const;
  ALWAYS_INLINE bool is_any_negative() const;
  ALWAYS_INLINE bool is_any_greater(const FourFloats &other) const;
  ALWAYS_INLINE bool is_any_greater_equal(const FourFloats &other) const;
  ALWAYS_INLINE bool is_any_less(const FourFloats &other) const;
  ALWAYS_INLINE bool is_any_less_equal(const FourFloats &other) const;
  ALWAYS_INLINE bool is_any_equal(const FourFloats &other) const;
  ALWAYS_INLINE bool is_any_not_equal(const FourFloats &other) const;

  ALWAYS_INLINE bool is_all_zero() const;
  ALWAYS_INLINE bool is_all_negative() const;
  ALWAYS_INLINE bool is_all_greater(const FourFloats &other) const;
  ALWAYS_INLINE bool is_all_greater_equal(const FourFloats &other) const;
  ALWAYS_INLINE bool is_all_less(const FourFloats &other) const;
  ALWAYS_INLINE bool is_all_less_equal(const FourFloats &other) const;
  ALWAYS_INLINE bool is_all_equal(const FourFloats &other) const;
  ALWAYS_INLINE bool is_all_not_equal(const FourFloats &other) const;

  ALWAYS_INLINE FourFloats blend(const FourFloats &other, const FourFloatsMask &mask) const;
  ALWAYS_INLINE FourFloats blend_zero(const FourFloatsMask &mask) const;

  ALWAYS_INLINE FourFloats madd(const FourFloats &m1, const FourFloats &m2) const;
  ALWAYS_INLINE FourFloats msub(const FourFloats &m1, const FourFloats &m2) const;

  ALWAYS_INLINE FourFloats min(const FourFloats &other) const;
  ALWAYS_INLINE FourFloats max(const FourFloats &other) const;
  ALWAYS_INLINE FourFloats sqrt() const;
  ALWAYS_INLINE FourFloats rsqrt() const;
  ALWAYS_INLINE FourFloats recip() const;

  ALWAYS_INLINE static const FourFloats &zero();
  ALWAYS_INLINE static const FourFloats &one();
  ALWAYS_INLINE static const FourFloats &negative_one();
  ALWAYS_INLINE static const FourFloats &two();
  ALWAYS_INLINE static const FourFloats &three();
  ALWAYS_INLINE static const FourFloats &four();
  ALWAYS_INLINE static const FourFloats &point_five();
  ALWAYS_INLINE static const FourFloats &flt_epsilon();

public:
  PN_vec4f _data;

  static FourFloats _zero;
  static FourFloats _one;
  static FourFloats _negative_one;
  static FourFloats _two;
  static FourFloats _three;
  static FourFloats _four;
  static FourFloats _point_five;
  static FourFloats _flt_epsilon;
};

ALWAYS_INLINE FourFloats simd_min(const FourFloats &a, const FourFloats &b);
ALWAYS_INLINE FourFloats simd_max(const FourFloats &a, const FourFloats &b);
ALWAYS_INLINE FourFloats simd_sqrt(const FourFloats &val);
ALWAYS_INLINE FourFloats simd_rsqrt(const FourFloats &val);
ALWAYS_INLINE FourFloats simd_recip(const FourFloats &val);
ALWAYS_INLINE void simd_transpose(FourFloats &a, FourFloats &b, FourFloats &c, FourFloats &d);

/**
 *
 */
class ALIGN_16BYTE FourVector3s : public SIMDVector3<FourFloats, FourVector3s> {
PUBLISHED:
  ALWAYS_INLINE FourVector3s() = default;
  ALWAYS_INLINE FourVector3s(const FourFloats &x, const FourFloats &y, const FourFloats &z);
  ALWAYS_INLINE FourVector3s(const LVecBase3f *vectors);
  ALWAYS_INLINE FourVector3s(const LVecBase3f &a, const LVecBase3f &b, const LVecBase3f &c, const LVecBase3f &d);
  ALWAYS_INLINE FourVector3s(const LVecBase3f &vec);

  ALWAYS_INLINE void load(const LVecBase3f *vectors);
  ALWAYS_INLINE void load(const LVecBase3f &a, const LVecBase3f &b, const LVecBase3f &c, const LVecBase3f &d);
  ALWAYS_INLINE void load(const LVecBase3f &fill);
};

/**
 *
 */
class ALIGN_16BYTE FourQuaternions : public SIMDQuaternion<FourFloats> {
PUBLISHED:
};

EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const FourFloats &ff);
EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const FourVector3s &ff);
//EXPCL_PANDA_MATHUTIL std::ostream &operator << (std::ostream &out, const FourQuaternions &ff);

#include "mathutil_sse_src.I"

