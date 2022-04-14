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

typedef __m256 PN_vec8f;
typedef __m256d PN_vec4d;
typedef __m256i PN_vec8i;

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
  ALWAYS_INLINE EightFloats operator | (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator ^ (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator - () const;

  ALWAYS_INLINE EightFloats operator > (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator >= (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator < (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator <= (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator == (const EightFloats &other) const;
  ALWAYS_INLINE EightFloats operator != (const EightFloats &other) const;

  ALWAYS_INLINE void operator *= (const EightFloats &other);
  ALWAYS_INLINE void operator /= (const EightFloats &other);
  ALWAYS_INLINE void operator -= (const EightFloats &other);
  ALWAYS_INLINE void operator += (const EightFloats &other);
  ALWAYS_INLINE void operator &= (const EightFloats &other);
  ALWAYS_INLINE void operator |= (const EightFloats &other);
  ALWAYS_INLINE void operator ^= (const EightFloats &other);

  ALWAYS_INLINE void operator = (const EightFloats &other);
  ALWAYS_INLINE void operator = (EightFloats &&other);

  ALWAYS_INLINE float operator [] (int n) const;
  ALWAYS_INLINE float &operator [] (int n);

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

/**
 *
 */
class ALIGN_32BYTE EightVector3s : public SIMDVector3<EightFloats> {
PUBLISHED:
};

/**
 *
 */
class ALIGN_32BYTE EightQuaternions : public SIMDQuaternion<EightFloats> {
PUBLISHED:
};

#include "mathutil_avx_src.I"
