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

/**
 *
 */
template<class FloatType, class Derived>
class SIMDVector3 {
PUBLISHED:
  ALWAYS_INLINE SIMDVector3() = default;
  ALWAYS_INLINE SIMDVector3(const FloatType &x, const FloatType &y, const FloatType &z);
  ALWAYS_INLINE SIMDVector3(const SIMDVector3 &copy);
  ALWAYS_INLINE SIMDVector3(SIMDVector3 &&other);

  ALWAYS_INLINE Derived cross(const SIMDVector3 &other) const;
  ALWAYS_INLINE FloatType dot(const SIMDVector3 &other) const;
  ALWAYS_INLINE void componentwise_mult(const SIMDVector3 &other);

  ALWAYS_INLINE Derived operator + (const SIMDVector3 &other) const;
  ALWAYS_INLINE Derived operator - (const SIMDVector3 &other) const;
  // Componentwise mult/div.
  ALWAYS_INLINE Derived operator * (const SIMDVector3 &other) const;
  ALWAYS_INLINE Derived operator / (const SIMDVector3 &other) const;
  // Scaling.
  ALWAYS_INLINE Derived operator * (const FloatType &scalar) const;
  ALWAYS_INLINE Derived operator / (const FloatType &scalar) const;
  // Vector negation.
  ALWAYS_INLINE Derived operator - () const;

  ALWAYS_INLINE void operator += (const SIMDVector3 &other);
  ALWAYS_INLINE void operator -= (const SIMDVector3 &other);
  ALWAYS_INLINE void operator *= (const SIMDVector3 &other);
  ALWAYS_INLINE void operator /= (const SIMDVector3 &other);
  ALWAYS_INLINE void operator *= (const FloatType &scalar);
  ALWAYS_INLINE void operator /= (const FloatType &scalar);

  ALWAYS_INLINE FloatType length() const;
  ALWAYS_INLINE FloatType length_squared() const;

  ALWAYS_INLINE void normalize();
  ALWAYS_INLINE Derived normalized() const;

  ALWAYS_INLINE const FloatType &get_x() const;
  ALWAYS_INLINE FloatType &get_x();
  ALWAYS_INLINE const FloatType &get_y() const;
  ALWAYS_INLINE FloatType &get_y();
  ALWAYS_INLINE const FloatType &get_z() const;
  ALWAYS_INLINE FloatType &get_z();

  ALWAYS_INLINE FloatType &operator [] (int n);
  ALWAYS_INLINE const FloatType &operator [] (int n) const;

protected:
  FloatType _v[3];
};

/**
 *
 */
template<class FloatType>
class SIMDQuaternion {
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

#if 0//defined(__AVX__) || defined(__AVX2__)
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
#define SIMD_NATIVE_WIDTH 8
typedef PN_vec8f PN_native_vecf;
typedef PN_vec8i PN_native_veci;
typedef PN_vec4d PN_native_vecd;
typedef EightFloats SIMDNativeFloat;
typedef EightVector3s SIMDNativeVector3;
typedef EightQuaternions SIMDNativeQuaternion;

#else

#define SIMD_NATIVE_ALIGNMENT 16
#define SIMD_NATIVE_WIDTH 4
typedef PN_vec4f PN_native_vecf;
typedef PN_vec4i PN_native_veci;
typedef PN_vec2d PN_native_vecd;
typedef FourFloats SIMDNativeFloat;
typedef FourVector3s SIMDNativeVector3;
typedef FourQuaternions SIMDNativeQuaternion;

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
