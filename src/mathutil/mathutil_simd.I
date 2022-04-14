/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_simd.I
 * @author brian
 * @date 2022-04-12
 */

/**
 * Initializes an SIMD vector from existing SIMD floats for each
 * component.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE SIMDVector3<FloatType, Derived>::
SIMDVector3(const FloatType &x, const FloatType &y, const FloatType &z) {
  _v[0] = x;
  _v[1] = y;
  _v[2] = z;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE SIMDVector3<FloatType, Derived>::
SIMDVector3(const SIMDVector3 &copy) {
  _v[0] = copy._v[0];
  _v[1] = copy._v[1];
  _v[2] = copy._v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE SIMDVector3<FloatType, Derived>::
SIMDVector3(SIMDVector3 &&other) {
  _v[0] = std::move(other._v[0]);
  _v[1] = std::move(other._v[1]);
  _v[2] = std::move(other._v[2]);
}

/**
 * Returns the cross product of this vector with the other vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
cross(const SIMDVector3 &other) const {
  FloatType x = _v[1] * other._v[2];
  x = x.msub(_v[2], other._v[1]);

  FloatType y = _v[2] * other._v[0];
  y = y.msub(_v[0], other._v[2]);

  FloatType z = _v[0] * other._v[1];
  z = z.msub(_v[1], other._v[0]);

  Derived result;
  result._v[0] = std::move(x);
  result._v[1] = std::move(y);
  result._v[2] = std::move(z);
  return result;
}

/**
 * Returns the dot product of this vector with the other vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType SIMDVector3<FloatType, Derived>::
dot(const SIMDVector3 &other) const {
  FloatType dot = _v[0] * other._v[0];
  dot = dot.madd(_v[1], other._v[1]);
  dot = dot.madd(_v[2], other._v[2]);
  return dot;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
componentwise_mult(const SIMDVector3 &other) {
  _v[0] *= other._v[0];
  _v[1] *= other._v[1];
  _v[2] *= other._v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator + (const SIMDVector3 &other) const {
  Derived result;
  result._v[0] = _v[0] + other._v[0];
  result._v[1] = _v[1] + other._v[1];
  result._v[2] = _v[2] + other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator - (const SIMDVector3 &other) const {
  Derived result;
  result._v[0] = _v[0] - other._v[0];
  result._v[1] = _v[1] - other._v[1];
  result._v[2] = _v[2] - other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator * (const SIMDVector3 &other) const {
  Derived result;
  result._v[0] = _v[0] * other._v[0];
  result._v[1] = _v[1] * other._v[1];
  result._v[2] = _v[2] * other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator / (const SIMDVector3 &other) const {
  Derived result;
  result._v[0] = _v[0] / other._v[0];
  result._v[1] = _v[1] / other._v[1];
  result._v[2] = _v[2] / other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator * (const FloatType &scalar) const {
  Derived result;
  result._v[0] = _v[0] * scalar;
  result._v[1] = _v[1] * scalar;
  result._v[2] = _v[2] * scalar;
  return result;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator / (const FloatType &scalar) const {
  Derived result;
  result._v[0] = _v[0] / scalar;
  result._v[1] = _v[1] / scalar;
  result._v[2] = _v[2] / scalar;
  return result;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
operator - () const {
  Derived result;
  result._v[0] = -_v[0];
  result._v[1] = -_v[1];
  result._v[2] = -_v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
operator += (const SIMDVector3 &other) {
  _v[0] += other._v[0];
  _v[1] += other._v[1];
  _v[2] += other._v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
operator -= (const SIMDVector3 &other) {
  _v[0] -= other._v[0];
  _v[1] -= other._v[1];
  _v[2] -= other._v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
operator *= (const SIMDVector3 &other) {
  _v[0] *= other._v[0];
  _v[1] *= other._v[1];
  _v[2] *= other._v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
operator /= (const SIMDVector3 &other) {
  _v[0] /= other._v[0];
  _v[1] /= other._v[1];
  _v[2] /= other._v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
operator *= (const FloatType &scalar) {
  _v[0] *= scalar;
  _v[1] *= scalar;
  _v[2] *= scalar;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
operator /= (const FloatType &scalar) {
  _v[0] /= scalar;
  _v[1] /= scalar;
  _v[2] /= scalar;
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType SIMDVector3<FloatType, Derived>::
length() const {
  return simd_sqrt(dot(*this));
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType SIMDVector3<FloatType, Derived>::
length_squared() const {
  return dot(*this);
}

/**
 * Normalizes the vector (or vectors in this case) in place.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE void SIMDVector3<FloatType, Derived>::
normalize() {
  FloatType len = length();
  _v[0] /= len;
  _v[1] /= len;
  _v[2] /= len;
}

/**
 * Returns a normalized version of this vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE Derived SIMDVector3<FloatType, Derived>::
normalized() const {
  FloatType len = length();
  Derived result;
  result._v[0] = _v[0] / len;
  result._v[1] = _v[1] / len;
  result._v[2] = _v[2] / len;
  return result;
}

/**
 * Returns the X component of the vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType, Derived>::
get_x() const {
  return _v[0];
}

/**
 * Returns the X component of the vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType, Derived>::
get_x() {
  return _v[0];
}

/**
 * Returns the Y component of the vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType, Derived>::
get_y() const {
  return _v[1];
}

/**
 * Returns the Y component of the vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType, Derived>::
get_y() {
  return _v[1];
}

/**
 * Returns the Z component of the vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType, Derived>::
get_z() const {
  return _v[2];
}

/**
 * Returns the Z component of the vector.
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType, Derived>::
get_z() {
  return _v[2];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType, Derived>::
operator [] (int n) {
  return _v[n];
}

/**
 *
 */
template<class FloatType, class Derived>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType, Derived>::
operator [] (int n) const {
  return _v[n];
}

/**
 * Returns the given value padded to be a multiple of alignment.
 *
 * Alignment must be a power of 2.
 */
INLINE int
simd_align_value(int value, int alignment) {
  nassertr((alignment % 2) == 0, value);
  return value + (-value & (alignment - 1));
}
