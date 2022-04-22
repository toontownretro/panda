/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_simd.h
 * @author brian
 * @date 2022-04-12
 */

#ifndef MATHUTIL_SIMD_H
#define MATHUTIL_SIMD_H

#include "pandabase.h"
#include "pnotify.h"
#include "bitMask.h"
#include "luse.h"
#include "pbitops.h"

#define SSE2      0
#define SSE4      1
#define AVX2_128  2
#define AVX2      3

/**
 * Template base class for a wrapper around a SIMD register+data type
 * combination.
 *
 * Provides operator overloading and various convenience wrapper methods.
 */
template<class Register, class Type>
class SIMDVector {
public:
  typedef SIMDVector<Register, Type> ThisClass;

  // Construction and assignment operators.
  ALWAYS_INLINE SIMDVector() = default;
  ALWAYS_INLINE SIMDVector(Type fill);
  ALWAYS_INLINE SIMDVector(Register &&data);
  ALWAYS_INLINE SIMDVector(const Register &data);
  ALWAYS_INLINE SIMDVector(const SIMDVector &copy);
  ALWAYS_INLINE SIMDVector(SIMDVector &&other);
  ALWAYS_INLINE void operator = (Type fill);
  ALWAYS_INLINE void operator = (Register &&data);
  ALWAYS_INLINE void operator = (const Register &data);
  ALWAYS_INLINE void operator = (const SIMDVector &copy);
  ALWAYS_INLINE void operator = (SIMDVector &&other);

  ALWAYS_INLINE static ThisClass load_aligned(const Type *data);
  ALWAYS_INLINE static ThisClass load_unaligned(const Type *data);
  ALWAYS_INLINE void load_aligned_in_place(const Type *data);
  ALWAYS_INLINE void load_unaligned_in_place(const Type *data);

  ALWAYS_INLINE static ThisClass blend(const ThisClass &a, const ThisClass &b, const ThisClass &mask);
  ALWAYS_INLINE ThisClass blend(const ThisClass &other, const ThisClass &mask) const;
  ALWAYS_INLINE void blend_in_place(const ThisClass &other, const ThisClass &mask);

  // Arithmetic operators.
  ALWAYS_INLINE ThisClass operator + (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator - (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator * (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator / (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator - () const;
  ALWAYS_INLINE void operator += (const ThisClass &other);
  ALWAYS_INLINE void operator -= (const ThisClass &other);
  ALWAYS_INLINE void operator *= (const ThisClass &other);
  ALWAYS_INLINE void operator /= (const ThisClass &other);
  ALWAYS_INLINE ThisClass madd(const ThisClass &m1, const ThisClass &m2) const;
  ALWAYS_INLINE ThisClass msub(const ThisClass &m1, const ThisClass &m2) const;
  ALWAYS_INLINE void madd_in_place(const ThisClass &m1, const ThisClass &m2);
  ALWAYS_INLINE void msub_in_place(const ThisClass &m1, const ThisClass &m2);

  // Logical operators.
  ALWAYS_INLINE ThisClass operator & (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator | (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator ^ (const ThisClass &other) const;
  ALWAYS_INLINE void operator &= (const ThisClass &other);
  ALWAYS_INLINE void operator |= (const ThisClass &other);
  ALWAYS_INLINE void operator ^= (const ThisClass &other);

  // Comparison operators.
  ALWAYS_INLINE ThisClass operator > (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator >= (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator < (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator <= (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator == (const ThisClass &other) const;
  ALWAYS_INLINE ThisClass operator != (const ThisClass &other) const;

  // Convenience methods to check if any or all columns compare a certain
  // way.
  ALWAYS_INLINE bool is_all_greater(const ThisClass &other) const;
  ALWAYS_INLINE bool is_all_greater_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_all_less(const ThisClass &other) const;
  ALWAYS_INLINE bool is_all_less_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_all_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_all_not_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_any_greater(const ThisClass &other) const;
  ALWAYS_INLINE bool is_any_greater_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_any_less(const ThisClass &other) const;
  ALWAYS_INLINE bool is_any_less_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_any_equal(const ThisClass &other) const;
  ALWAYS_INLINE bool is_any_not_equal(const ThisClass &other) const;

  // Methods to query mask returned from a comparison between two
  // vectors.
  ALWAYS_INLINE bool is_all_on() const;
  ALWAYS_INLINE bool is_all_off() const;
  ALWAYS_INLINE bool is_any_on() const;
  ALWAYS_INLINE bool is_any_off() const;
  ALWAYS_INLINE int get_num_on_bits() const;

  ALWAYS_INLINE const Register &operator * () const;
  ALWAYS_INLINE Register &operator * ();

  ALWAYS_INLINE operator Register () const;

  Type &operator [] (int n);
  const Type &operator [] (int n) const;
  Type *get_data();
  const Type *get_data() const;

  static constexpr int num_columns = sizeof(Register) / sizeof(Type);
  ALWAYS_INLINE static constexpr int get_num_columns();

  ALWAYS_INLINE std::ostream &output(std::ostream &out) const;

private:
  Register _data;
};

/**
 * Structure-of-arrays 3-component vector, SIMD accelerated.
 *
 * Allows performing vector operations on multiple vectors at the same time.
 */
template<class FloatType>
class SIMDVector3 {
public:
  typedef SIMDVector3<FloatType> ThisClass;

  ALWAYS_INLINE SIMDVector3() = default;
  ALWAYS_INLINE SIMDVector3(const FloatType &x, const FloatType &y, const FloatType &z);
  ALWAYS_INLINE SIMDVector3(const LVecBase3f &fill);
  ALWAYS_INLINE SIMDVector3(const SIMDVector3 &copy);
  ALWAYS_INLINE SIMDVector3(SIMDVector3 &&other);

  ALWAYS_INLINE void operator = (const SIMDVector3 &other);
  ALWAYS_INLINE void operator = (SIMDVector3 &&other);
  ALWAYS_INLINE void operator = (const LVecBase3f &fill);

  ALWAYS_INLINE void set(const FloatType &x, const FloatType &y, const FloatType &z);
  ALWAYS_INLINE void fill(const LVecBase3f &vec);

  ALWAYS_INLINE ThisClass cross(const SIMDVector3 &other) const;
  ALWAYS_INLINE FloatType dot(const SIMDVector3 &other) const;
  ALWAYS_INLINE void componentwise_mult(const SIMDVector3 &other);

  ALWAYS_INLINE ThisClass madd(const SIMDVector3 &m1, const FloatType &m2) const;
  ALWAYS_INLINE void madd_in_place(const SIMDVector3 &m1, const FloatType &m2);
  ALWAYS_INLINE ThisClass msub(const SIMDVector3 &m1, const FloatType &m2) const;
  ALWAYS_INLINE void msub_in_place(const SIMDVector3 &m1, const FloatType &m2);

  ALWAYS_INLINE ThisClass operator + (const SIMDVector3 &other) const;
  ALWAYS_INLINE ThisClass operator - (const SIMDVector3 &other) const;
  // Componentwise mult/div.
  ALWAYS_INLINE ThisClass operator * (const SIMDVector3 &other) const;
  ALWAYS_INLINE ThisClass operator / (const SIMDVector3 &other) const;
  // Scaling.
  ALWAYS_INLINE ThisClass operator * (const FloatType &scalar) const;
  ALWAYS_INLINE ThisClass operator / (const FloatType &scalar) const;
  // Vector negation.
  ALWAYS_INLINE ThisClass operator - () const;

  ALWAYS_INLINE void operator += (const SIMDVector3 &other);
  ALWAYS_INLINE void operator -= (const SIMDVector3 &other);
  ALWAYS_INLINE void operator *= (const SIMDVector3 &other);
  ALWAYS_INLINE void operator /= (const SIMDVector3 &other);
  ALWAYS_INLINE void operator *= (const FloatType &scalar);
  ALWAYS_INLINE void operator /= (const FloatType &scalar);

  ALWAYS_INLINE FloatType length() const;
  ALWAYS_INLINE FloatType length_squared() const;

  ALWAYS_INLINE void normalize();
  ALWAYS_INLINE ThisClass normalized() const;

  ALWAYS_INLINE const FloatType &get_x() const;
  ALWAYS_INLINE FloatType &get_x();
  ALWAYS_INLINE const FloatType &get_y() const;
  ALWAYS_INLINE FloatType &get_y();
  ALWAYS_INLINE const FloatType &get_z() const;
  ALWAYS_INLINE FloatType &get_z();

  ALWAYS_INLINE LVecBase3f get_lvec(int n) const;
  ALWAYS_INLINE void get_lvec(int n, LVecBase3f &vec) const;

  ALWAYS_INLINE void set_lvec(int n, const LVecBase3f &vec);

  ALWAYS_INLINE FloatType &operator [] (int n);
  ALWAYS_INLINE const FloatType &operator [] (int n) const;

  static constexpr int num_vectors = FloatType::num_columns;
  ALWAYS_INLINE static constexpr int get_num_vectors();

  ALWAYS_INLINE std::ostream &output(std::ostream &out) const;

protected:
  FloatType _v[3];
};

/**
 * Structure-of-arrays quaternion, SIMD accelerated.
 *
 * Allows performing quaternion operations on multiple quats at the same time.
 */
template<class FloatType>
class SIMDQuaternion {
public:
  typedef SIMDQuaternion<FloatType> ThisClass;
  typedef SIMDVector3<FloatType> VectorType;

  ALWAYS_INLINE SIMDQuaternion() = default;
  ALWAYS_INLINE SIMDQuaternion(const FloatType &r, const FloatType &i, const FloatType &j, const FloatType &k);
  ALWAYS_INLINE SIMDQuaternion(const LQuaternionf &fill);
  ALWAYS_INLINE SIMDQuaternion(const SIMDQuaternion &copy);
  ALWAYS_INLINE SIMDQuaternion(SIMDQuaternion &&other);

  ALWAYS_INLINE void operator = (const LQuaternionf &fill);
  ALWAYS_INLINE void operator = (const SIMDQuaternion &copy);
  ALWAYS_INLINE void operator = (SIMDQuaternion &&other);

  ALWAYS_INLINE ThisClass operator * (const SIMDQuaternion &other) const;
  ALWAYS_INLINE void operator *= (const SIMDQuaternion &other);
  ALWAYS_INLINE ThisClass operator - () const;

  ALWAYS_INLINE FloatType dot(const SIMDQuaternion &other) const;

  ALWAYS_INLINE ThisClass scale_angle(const FloatType &scale) const;
  ALWAYS_INLINE ThisClass accumulate(const SIMDQuaternion &other) const;
  ALWAYS_INLINE ThisClass accumulate_source(const SIMDQuaternion &other) const;
  ALWAYS_INLINE ThisClass accumulate_scaled_rhs(const SIMDQuaternion &rhs, const FloatType &rhs_scale) const;
  ALWAYS_INLINE ThisClass accumulate_scaled_rhs_source(const SIMDQuaternion &rhs, const FloatType &rhs_scale) const;
  ALWAYS_INLINE ThisClass accumulate_scaled_lhs(const SIMDQuaternion &rhs, const FloatType &lhs_scale) const;
  ALWAYS_INLINE ThisClass accumulate_scaled_lhs_source(const SIMDQuaternion &rhs, const FloatType &rhs_scale) const;

  ALWAYS_INLINE ThisClass align(const SIMDQuaternion &other) const;
  ALWAYS_INLINE void align_in_place(const SIMDQuaternion &other);

  ALWAYS_INLINE ThisClass normalized() const;
  ALWAYS_INLINE void normalize();

  ALWAYS_INLINE ThisClass lerp(const SIMDQuaternion &other, const FloatType &frac) const;
  ALWAYS_INLINE ThisClass align_lerp(const SIMDQuaternion &other, const FloatType &frac) const;
  ALWAYS_INLINE ThisClass slerp(const SIMDQuaternion &other, const FloatType &frac) const;
  ALWAYS_INLINE ThisClass align_slerp(const SIMDQuaternion &other, const FloatType &frac) const;

  ALWAYS_INLINE LQuaternionf get_lquat(int n) const;
  ALWAYS_INLINE void get_lquat(int n, LQuaternionf &quat) const;

  ALWAYS_INLINE void set_lquat(int n, const LQuaternionf &quat);

  ALWAYS_INLINE FloatType &operator [] (int n);
  ALWAYS_INLINE const FloatType &operator [] (int n) const;

  static constexpr int num_quats = FloatType::num_columns;
  ALWAYS_INLINE static constexpr int get_num_quats();

  ALWAYS_INLINE std::ostream &output(std::ostream &out) const;

protected:
  // real, i, j, k
  FloatType _v[4];
};

#if !defined(PLATFORM_64BIT) && !defined(__SSE2__)
#pragma error("SSE2 is a minimum requirement for mathutil_simd!")
#endif

#ifndef HAVE_SSE2
#define HAVE_SSE2 1
#endif

#include "mathutil_sse_src.h"

#if defined(__AVX__) || defined(__AVX2__)
#define HAVE_AVX 1
#include "mathutil_avx_src.h"
#endif

#if 0//defined(__AVX512F__)
#define HAVE_AVX512 1
#include "mathutil_avx512_src.h"
#endif

#if defined(HAVE_AVX512)

#define SIMD_NATIVE_ALIGNMENT 64
#define SIMD_NATIVE_WIDTH 16
typedef PN_vec16f PN_native_vecf;
typedef PN_vec16i PN_native_veci;
typedef PN_vec8d PN_native_vecd;
typedef SixteenFloats SIMDNativeFloat;
typedef SixteenVector3s SIMDNativeVector3;
typedef SixteenQuaternions SIMDNativeQuaternion;

#elif defined(HAVE_AVX)

#define SIMD_NATIVE_ALIGNMENT 32
typedef PN_vec8f PN_native_vecf;
typedef PN_vec8i PN_native_veci;
typedef AVXFloatVector SIMDFloatVector;
typedef AVXVector3f SIMDVector3f;
typedef AVXQuaternionf SIMDQuaternionf;

#else

#define SIMD_NATIVE_ALIGNMENT 16
typedef PN_vec4f PN_native_vecf;
typedef PN_vec4i PN_native_veci;
typedef SSEFloatVector SIMDFloatVector;
typedef SSEVector3f SIMDVector3f;
typedef SSEQuaternionf SIMDQuaternionf;

#endif

#ifdef CPPPARSER
#define SIMD_ALIGN(x)
#elif defined(_MSC_VER)
#define SIMD_ALIGN(x) __declspec(align(x))
#elif defined(__GNUC__)
#define SIMD_ALIGN(x) __attribute__ ((aligned (x)))
#else
#define SIMD_ALIGN(x)
#endif
#define SIMD_NATIVE_ALIGN SIMD_ALIGN(SIMD_NATIVE_ALIGNMENT)

INLINE int simd_align_value(int value, int alignment = SIMD_NATIVE_ALIGNMENT);

#include "mathutil_simd.I"

#endif // MATHUTIL_SIMD_H
