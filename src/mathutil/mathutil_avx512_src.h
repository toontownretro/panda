/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_avx512_src.h
 * @author brian
 * @date 2022-04-13
 */

#include "pandabase.h"
#include <zmmintrin.h>
#include "bitMask.h"

typedef __m512 PN_vec16f;
typedef __m512d PN_vec8d;
typedef __m512i PN_vec16i;

/**
 * Sixteen 32-bit floats occupying a single M512 SIMD register.
 *
 * AVX512
 */
class EXPCL_PANDA_MATHUTIL ALIGN_64BYTE SixteenFloats {
PUBLISHED:
  static const int width = 16;

  ALWAYS_INLINE SixteenFloats() = default;
  ALWAYS_INLINE SixteenFloats(float fill);
  ALWAYS_INLINE SixteenFloats(float a, float b, float c, float d,
                              float e, float f, float g, float h,
                              float i, float j, float k, float l,
                              float m, float n, float o, float p);
  ALWAYS_INLINE SixteenFloats(const float *data, bool aligned);
  ALWAYS_INLINE SixteenFloats(const PN_vec16f &data);
  ALWAYS_INLINE SixteenFloats(PN_vec16f &&data);
  ALWAYS_INLINE SixteenFloats(const SixteenFloats &other);
  ALWAYS_INLINE SixteenFloats(SixteenFloats &&other);

  ALWAYS_INLINE void load();
  ALWAYS_INLINE void load(float fill);
  ALWAYS_INLINE void load(float a, float b, float c, float d,
                          float e, float f, float g, float h,
                          float i, float j, float k, float l,
                          float m, float n, float o, float p);
  ALWAYS_INLINE void load(const float *data);
  ALWAYS_INLINE void load_unaligned(const float *data);

  ALWAYS_INLINE SixteenFloats operator * (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator / (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator - (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator + (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator & (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator | (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator ^ (const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats operator - () const;

  ALWAYS_INLINE BitMask32 operator > (const SixteenFloats &other) const;
  ALWAYS_INLINE BitMask32 operator >= (const SixteenFloats &other) const;
  ALWAYS_INLINE BitMask32 operator < (const SixteenFloats &other) const;
  ALWAYS_INLINE BitMask32 operator <= (const SixteenFloats &other) const;
  ALWAYS_INLINE BitMask32 operator == (const SixteenFloats &other) const;
  ALWAYS_INLINE BitMask32 operator != (const SixteenFloats &other) const;

  ALWAYS_INLINE void operator *= (const SixteenFloats &other);
  ALWAYS_INLINE void operator /= (const SixteenFloats &other);
  ALWAYS_INLINE void operator -= (const SixteenFloats &other);
  ALWAYS_INLINE void operator += (const SixteenFloats &other);
  ALWAYS_INLINE void operator &= (const SixteenFloats &other);
  ALWAYS_INLINE void operator |= (const SixteenFloats &other);
  ALWAYS_INLINE void operator ^= (const SixteenFloats &other);

  ALWAYS_INLINE void operator = (const SixteenFloats &other);
  ALWAYS_INLINE void operator = (SixteenFloats &&other);

  ALWAYS_INLINE float operator [] (int n) const;
  ALWAYS_INLINE float &operator [] (int n);

  ALWAYS_INLINE bool is_any_zero() const;
  ALWAYS_INLINE bool is_any_negative() const;
  ALWAYS_INLINE bool is_any_greater(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_any_greater_equal(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_any_less(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_any_less_equal(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_any_equal(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_any_not_equal(const SixteenFloats &other) const;

  ALWAYS_INLINE bool is_all_zero() const;
  ALWAYS_INLINE bool is_all_negative() const;
  ALWAYS_INLINE bool is_all_greater(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_all_greater_equal(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_all_less(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_all_less_equal(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_all_equal(const SixteenFloats &other) const;
  ALWAYS_INLINE bool is_all_not_equal(const SixteenFloats &other) const;

  ALWAYS_INLINE SixteenFloats min(const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats max(const SixteenFloats &other) const;
  ALWAYS_INLINE SixteenFloats sqrt() const;
  ALWAYS_INLINE SixteenFloats rsqrt() const;
  ALWAYS_INLINE SixteenFloats recip() const;

  ALWAYS_INLINE static const SixteenFloats &zero();
  ALWAYS_INLINE static const SixteenFloats &one();
  ALWAYS_INLINE static const SixteenFloats &negative_one();
  ALWAYS_INLINE static const SixteenFloats &two();
  ALWAYS_INLINE static const SixteenFloats &three();
  ALWAYS_INLINE static const SixteenFloats &four();
  ALWAYS_INLINE static const SixteenFloats &point_five();
  ALWAYS_INLINE static const SixteenFloats &flt_epsilon();

public:
  PN_vec16f _data;

  static SixteenFloats _zero;
  static SixteenFloats _one;
  static SixteenFloats _negative_one;
  static SixteenFloats _two;
  static SixteenFloats _three;
  static SixteenFloats _four;
  static SixteenFloats _point_five;
  static SixteenFloats _flt_epsilon;
};

ALWAYS_INLINE SixteenFloats simd_min(const SixteenFloats &a, const SixteenFloats &b);
ALWAYS_INLINE SixteenFloats simd_max(const SixteenFloats &a, const SixteenFloats &b);
ALWAYS_INLINE SixteenFloats simd_sqrt(const SixteenFloats &val);
ALWAYS_INLINE SixteenFloats simd_rsqrt(const SixteenFloats &val);
ALWAYS_INLINE SixteenFloats simd_recip(const SixteenFloats &val);

/**
 *
 */
class ALIGN_64BYTE SixteenVector3s : public SIMDVector3<SixteenFloats> {
PUBLISHED:
};

/**
 *
 */
class ALIGN_64BYTE SixteenQuaternions : public SIMDQuaternion<SixteenFloats> {
PUBLISHED:
};

#include "mathutil_avx512_src.I"
