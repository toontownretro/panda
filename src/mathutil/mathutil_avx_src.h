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

#include <immintrin.h>

#ifdef CPPPARSER
typedef int __m256;
typedef int __m256d;
typedef int __m256i;
typedef int __mmask8;
#endif

#if defined(__AVX512F__) && defined(__AVX512VL__)
// With AVX-512, we can compare directly into a mask.
#include <zmmintrin.h>
#ifndef HAVE_MASK_REGISTERS
#define HAVE_MASK_REGISTERS 1
#endif
typedef __mmask8 PN_vec8f_mask;
typedef __mmask8 PN_vec4d_mask;
typedef __mmask8 PN_vec8i_mask;
#else
typedef __m256 PN_vec8f_mask;
typedef __m256d PN_vec4d_mask;
typedef __m256i PN_vec8i_mask;
#endif

typedef __m256 PN_vec8f;
typedef __m256d PN_vec4d;
typedef __m256i PN_vec8i;

/**
 * Returned from comparisons between two EightFloats.
 *
 * With AVX-512, this is an actual mask in a mask register with a bit set
 * for each column that evaluated true.
 *
 * Without AVX-512, this is a vector storing ~0 for true, and 0 for false,
 * in each column.
 */
class EightFloatsMask {
PUBLISHED:
  EightFloatsMask() = delete;
  EightFloatsMask(const PN_vec8f_mask &mask) = delete;
  void operator = (const PN_vec8f_mask &mask) = delete;

  ALWAYS_INLINE EightFloatsMask(PN_vec8f_mask &&mask);
  void operator = (PN_vec8f_mask &&mask);

  ALWAYS_INLINE EightFloatsMask operator & (const EightFloatsMask &other) const;
  ALWAYS_INLINE EightFloatsMask operator | (const EightFloatsMask &other) const;
  ALWAYS_INLINE EightFloatsMask operator ^ (const EightFloatsMask &other) const;
  ALWAYS_INLINE EightFloatsMask operator ~ () const;

  ALWAYS_INLINE bool is_all_on() const;
  ALWAYS_INLINE bool is_all_off() const;

public:
  PN_vec8f_mask _mask;
};

/**
 * Eight 32-bit floats occupying a single M256 SIMD register.
 *
 * AVX
 */
class EXPCL_PANDA_MATHUTIL ALIGN_32BYTE EightFloats {
PUBLISHED:
  static const int width = 8;

  ALWAYS_INLINE EightFloats() = default;
  ALWAYS_INLINE EightFloats(float fill);
  ALWAYS_INLINE EightFloats(float a, float b, float c, float d,
                            float e, float f, float g, float h);
  ALWAYS_INLINE EightFloats(const float *data, bool aligned);
  ALWAYS_INLINE EightFloats(const PN_vec8f &data);
  ALWAYS_INLINE EightFloats(PN_vec8f &&data);
  ALWAYS_INLINE EightFloats(const EightFloats &other);
  ALWAYS_INLINE EightFloats(EightFloats &&other);

  ALWAYS_INLINE void load();
  ALWAYS_INLINE void load(float fill);
  ALWAYS_INLINE void load(float a, float b, float c, float d,
                          float e, float f, float g, float h);
  ALWAYS_INLINE void load(const float *data);
  ALWAYS_INLINE void load_unaligned(const float *data);

  ALWAYS_INLINE EightFloats operator * (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator / (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator - (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator + (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator & (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator & (const EightFloatsMask &other) const;
  ALWAYS_INLINE EightFloats operator | (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator ^ (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator - () const;

  ALWAYS_INLINE EightFloatsMask operator > (const EightFloats &other) const;
  ALWAYS_INLINE EightFloatsMask operator >= (const EightFloats &other) const;
  ALWAYS_INLINE EightFloatsMask operator < (const EightFloats &other) const;
  ALWAYS_INLINE EightFloatsMask operator <= (const EightFloats &other) const;
  ALWAYS_INLINE EightFloatsMask operator == (const EightFloats &other) const;
  ALWAYS_INLINE EightFloatsMask operator != (const EightFloats &other) const;

  ALWAYS_INLINE void operator *= (const EightFloats &other);
  ALWAYS_INLINE void operator /= (const EightFloats &other);
  ALWAYS_INLINE void operator -= (const EightFloats &other);
  ALWAYS_INLINE void operator += (const EightFloats &other);
  ALWAYS_INLINE void operator &= (const EightFloats &other);
  ALWAYS_INLINE void operator |= (const EightFloats &other);
  ALWAYS_INLINE void operator ^= (const EightFloats &other);

  ALWAYS_INLINE const PN_vec8f &operator * () const;

  ALWAYS_INLINE void operator = (const EightFloats &other);
  ALWAYS_INLINE void operator = (EightFloats &&other);

  ALWAYS_INLINE float operator [] (int n) const;
  ALWAYS_INLINE float &operator [] (int n);
  ALWAYS_INLINE float *modify_data();
  ALWAYS_INLINE const float *get_data() const;

  ALWAYS_INLINE bool is_any_zero() const;
  ALWAYS_INLINE bool is_any_negative() const;
  ALWAYS_INLINE bool is_any_greater(const EightFloats &other) const;
  ALWAYS_INLINE bool is_any_greater_equal(const EightFloats &other) const;
  ALWAYS_INLINE bool is_any_less(const EightFloats &other) const;
  ALWAYS_INLINE bool is_any_less_equal(const EightFloats &other) const;
  ALWAYS_INLINE bool is_any_equal(const EightFloats &other) const;
  ALWAYS_INLINE bool is_any_not_equal(const EightFloats &other) const;

  ALWAYS_INLINE bool is_all_zero() const;
  ALWAYS_INLINE bool is_all_negative() const;
  ALWAYS_INLINE bool is_all_greater(const EightFloats &other) const;
  ALWAYS_INLINE bool is_all_greater_equal(const EightFloats &other) const;
  ALWAYS_INLINE bool is_all_less(const EightFloats &other) const;
  ALWAYS_INLINE bool is_all_less_equal(const EightFloats &other) const;
  ALWAYS_INLINE bool is_all_equal(const EightFloats &other) const;
  ALWAYS_INLINE bool is_all_not_equal(const EightFloats &other) const;

  ALWAYS_INLINE EightFloats blend(const EightFloats &other, const EightFloatsMask &mask) const;
  ALWAYS_INLINE EightFloats blend_zero(const EightFloatsMask &mask) const;

  ALWAYS_INLINE EightFloats madd(const EightFloats &m1, const EightFloats &m2) const;
  ALWAYS_INLINE EightFloats msub(const EightFloats &m1, const EightFloats &m2) const;

  ALWAYS_INLINE EightFloats min(const EightFloats &other) const;
  ALWAYS_INLINE EightFloats max(const EightFloats &other) const;
  ALWAYS_INLINE EightFloats sqrt() const;
  ALWAYS_INLINE EightFloats rsqrt() const;
  ALWAYS_INLINE EightFloats recip() const;

  ALWAYS_INLINE static const EightFloats &zero();
  ALWAYS_INLINE static const EightFloats &one();
  ALWAYS_INLINE static const EightFloats &negative_one();
  ALWAYS_INLINE static const EightFloats &two();
  ALWAYS_INLINE static const EightFloats &three();
  ALWAYS_INLINE static const EightFloats &four();
  ALWAYS_INLINE static const EightFloats &point_five();
  ALWAYS_INLINE static const EightFloats &flt_epsilon();

public:
  PN_vec8f _data;

  static EightFloats _zero;
  static EightFloats _one;
  static EightFloats _negative_one;
  static EightFloats _two;
  static EightFloats _three;
  static EightFloats _four;
  static EightFloats _point_five;
  static EightFloats _flt_epsilon;
};

ALWAYS_INLINE EightFloats simd_min(const EightFloats &a, const EightFloats &b);
ALWAYS_INLINE EightFloats simd_max(const EightFloats &a, const EightFloats &b);
ALWAYS_INLINE EightFloats simd_sqrt(const EightFloats &val);
ALWAYS_INLINE EightFloats simd_rsqrt(const EightFloats &val);
ALWAYS_INLINE EightFloats simd_recip(const EightFloats &val);
ALWAYS_INLINE void simd_transpose(EightFloats &a, EightFloats &b, EightFloats &c, EightFloats &d);

/**
 *
 */
class ALIGN_32BYTE EightVector3s : public SIMDVector3<EightFloats, EightVector3s> {
PUBLISHED:
  ALWAYS_INLINE EightVector3s() = default;
  ALWAYS_INLINE EightVector3s(const EightFloats &x, const EightFloats &y, const EightFloats &z);
  ALWAYS_INLINE EightVector3s(const LVecBase3f *vectors);
  //ALWAYS_INLINE EightVector3s(const LVecBase3f &a, const LVecBase3f &b, const LVecBase3f &c, const LVecBase3f &d);
  ALWAYS_INLINE EightVector3s(const LVecBase3f &vec);

  ALWAYS_INLINE void load(const LVecBase3f *vectors);
  //ALWAYS_INLINE void load(const LVecBase3f &a, const LVecBase3f &b, const LVecBase3f &c, const LVecBase3f &d);
  ALWAYS_INLINE void load(const LVecBase3f &fill);
};

/**
 *
 */
class ALIGN_32BYTE EightQuaternions : public SIMDQuaternion<EightFloats, EightQuaternions> {
PUBLISHED:
};

#include "mathutil_avx_src.I"
