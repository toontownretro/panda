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
 * Initializes the SIMD register with the same given value
 * in all columns.
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type>::
SIMDVector(Type fill) {
  simd_fill(_data, fill);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type>::
SIMDVector(Register &&data) :
  _data(std::move(data))
{
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type>::
SIMDVector(const Register &data) :
  _data(data)
{
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type>::
SIMDVector(const SIMDVector &copy) :
  _data(copy._data)
{
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type>::
SIMDVector(SIMDVector &&other) :
  _data(std::move(other._data))
{
}

/**
 * Sets the register to contain the given value in all columns.
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator = (Type fill) {
  simd_fill(_data, fill);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator = (Register &&data) {
  _data = std::move(data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator = (const Register &data) {
  _data = data;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator = (const SIMDVector &copy) {
  _data = copy._data;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator = (SIMDVector &&other) {
  _data = std::move(other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
load_aligned(const Type *data) {
  ThisClass ret;
  simd_load_aligned(ret._data, data);
  return ret;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
load_unaligned(const Type *data) {
  ThisClass ret;
  simd_load_unaligned(ret._data, data);
  return ret;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
load_aligned_in_place(const Type *data) {
  simd_load_aligned(_data, data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
load_unaligned_in_place(const Type *data) {
  simd_load_unaligned(_data, data);
}

/**
 * Returns a new vector that selects from `a` where the bit is not set, and
 * from `b` where the bit is set.
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
blend(const SIMDVector &a, const SIMDVector &b, const SIMDVector &mask) {
  return simd_blend(mask._data, a._data, b._data);
}

/**
 * Returns a new vector that selects from this vector where the bit is not
 * set, and from the other vector where the bit is set.
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
blend(const SIMDVector &other, const SIMDVector &mask) const {
  return simd_blend(mask._data, _data, other._data);
}

/**
 * Sets this vector to contain the original value where the bit in `mask`
 * is not set, and the value from `other` where the bit is set.
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
blend_in_place(const SIMDVector &other, const SIMDVector &mask) {
  _data = simd_blend(mask._data, _data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator + (const ThisClass &other) const {
  return simd_add(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator - (const ThisClass &other) const {
  return simd_sub(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator * (const ThisClass &other) const {
  return simd_mul(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator / (const ThisClass &other) const {
  return simd_div(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator - () const {
  return simd_neg(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator += (const ThisClass &other) {
  _data = simd_add(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator -= (const ThisClass &other) {
  _data = simd_sub(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator *= (const ThisClass &other) {
  _data = simd_mul(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator /= (const ThisClass &other) {
  _data = simd_div(_data, other._data);
}

/**
 * Multiplies vectors m1 and m2 together, and returns the result added onto
 * this vector.
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
madd(const ThisClass &m1, const ThisClass &m2) const {
  return simd_madd(m1._data, m2._data, _data);
}

/**
 * Multiplies vectors m1 and m2 together, and returns the result subtracted
 * from this vector.
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
msub(const ThisClass &m1, const ThisClass &m2) const {
  return simd_msub(m1._data, m2._data, _data);
}

/**
 * Multiplies vectors m1 and m2 together, and adds the result onto this
 * vector, as an in-place operation.
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
madd_in_place(const ThisClass &m1, const ThisClass &m2) {
  _data = simd_madd(m1._data, m2._data, _data);
}

/**
 * Multiplies vectors m1 and m2 together, and subtracts the result from this
 * vector, as an in-place operation.
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
msub_in_place(const ThisClass &m1, const ThisClass &m2) {
  _data = simd_msub(m1._data, m2._data, _data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator & (const ThisClass &other) const {
  return simd_and(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator | (const ThisClass &other) const {
  return simd_or(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator ^ (const ThisClass &other) const {
  return simd_xor(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator &= (const ThisClass &other) {
  _data = simd_and(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator |= (const ThisClass &other) {
  _data = simd_or(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE void SIMDVector<Register, Type>::
operator ^= (const ThisClass &other) {
  _data = simd_xor(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator > (const ThisClass &other) const {
  return simd_cmp_greater(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator >= (const ThisClass &other) const {
  return simd_cmp_greater_equal(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator < (const ThisClass &other) const {
  return simd_cmp_less(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator <= (const ThisClass &other) const {
  return simd_cmp_less_equal(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator == (const ThisClass &other) const {
  return simd_cmp_equal(_data, other._data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type> SIMDVector<Register, Type>::
operator != (const ThisClass &other) const {
  return simd_cmp_not_equal(_data, other._data);
}

/**
 * Returns true if all columns of this vector are greater than the
 * corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_greater(const ThisClass &other) const {
  return simd_is_all_greater(_data, other._data);
}

/**
 * Returns true if all columns of this vector are greater than or equal
 * the to corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_greater_equal(const ThisClass &other) const {
  return simd_is_all_greater_equal(_data, other._data);
}

/**
 * Returns true if all columns of this vector are less than the
 * corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_less(const ThisClass &other) const {
  return simd_is_all_less(_data, other._data);
}

/**
 * Returns true if all columns of this vector are less than or equal
 * to the corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_less_equal(const ThisClass &other) const {
  return simd_is_all_less_equal(_data, other._data);
}

/**
 * Returns true if all columns of this vector are equal
 * to corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_equal(const ThisClass &other) const {
  return simd_is_all_equal(_data, other._data);
}

/**
 * Returns true if all columns of this vector are not equal
 * to corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_not_equal(const ThisClass &other) const {
  return simd_is_all_not_equal(_data, other._data);
}

/**
 * Returns true if any columns of this vector are greater than the
 * corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_greater(const ThisClass &other) const {
  return simd_is_any_greater(_data, other._data);
}

/**
 * Returns true if any columns of this vector are greater than or equal
 * the to corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_greater_equal(const ThisClass &other) const {
  return simd_is_any_greater_equal(_data, other._data);
}

/**
 * Returns true if any columns of this vector are less than the
 * corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_less(const ThisClass &other) const {
  return simd_is_any_less(_data, other._data);
}

/**
 * Returns true if any columns of this vector are less than or equal
 * to the corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_less_equal(const ThisClass &other) const {
  return simd_is_any_less_equal(_data, other._data);
}

/**
 * Returns true if any columns of this vector are equal
 * to corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_equal(const ThisClass &other) const {
  return simd_is_any_equal(_data, other._data);
}

/**
 * Returns true if any columns of this vector are not equal
 * to corresponding columns of the other vector.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_not_equal(const ThisClass &other) const {
  return simd_is_any_not_equal(_data, other._data);
}

/**
 * Returns true if all columns have the high bit set, indicating that all
 * columns passed the comparison.
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_on() const {
  return simd_is_all_on(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_all_off() const {
  return simd_is_all_off(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_on() const {
  return simd_is_any_on(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE bool SIMDVector<Register, Type>::
is_any_off() const {
  return simd_is_any_off(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE int SIMDVector<Register, Type>::
get_num_on_bits() const {
  return count_bits_in_word(simd_test_sign(_data));
}

/**
 * Returns the number of columns in the vector; the number of data elements
 * operated on simultaneously by a single instruction.
 */
template<class Register, class Type>
ALWAYS_INLINE constexpr int SIMDVector<Register, Type>::
get_num_columns() {
  return num_columns;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE const Register &SIMDVector<Register, Type>::
operator * () const {
  return _data;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE Register &SIMDVector<Register, Type>::
operator * () {
  return _data;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE SIMDVector<Register, Type>::
operator Register () const {
  return _data;
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE Type &SIMDVector<Register, Type>::
operator [] (int n) {
  return simd_col(_data, n);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE const Type &SIMDVector<Register, Type>::
operator [] (int n) const {
  return simd_col(_data, n);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE Type *SIMDVector<Register, Type>::
get_data() {
  return simd_data(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE const Type *SIMDVector<Register, Type>::
get_data() const {
  return simd_data(_data);
}

/**
 *
 */
template<class Register, class Type>
ALWAYS_INLINE std::ostream &SIMDVector<Register, Type>::
output(std::ostream &out) const {
  out << _data;
  return out;
}

/**
 * Initializes an SIMD vector from existing SIMD floats for each
 * component.
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType>::
SIMDVector3(const FloatType &x, const FloatType &y, const FloatType &z) {
  _v[0] = x;
  _v[1] = y;
  _v[2] = z;
}

/**
 * Initializes an SIMD vector3 from a single linmath vec3.
 *
 * Each component of the linmath vector is duplicated to each column
 * of the SIMD registers for X Y and Z.
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType>::
SIMDVector3(const LVecBase3f &fill) {
  _v[0] = fill[0];
  _v[1] = fill[1];
  _v[2] = fill[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType>::
SIMDVector3(const SIMDVector3 &copy) {
  _v[0] = copy._v[0];
  _v[1] = copy._v[1];
  _v[2] = copy._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType>::
SIMDVector3(SIMDVector3 &&other) {
  _v[0] = std::move(other._v[0]);
  _v[1] = std::move(other._v[1]);
  _v[2] = std::move(other._v[2]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator = (const SIMDVector3 &other) {
  _v[0] = other._v[0];
  _v[1] = other._v[1];
  _v[2] = other._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator = (SIMDVector3 &&other) {
  _v[0] = std::move(other._v[0]);
  _v[1] = std::move(other._v[1]);
  _v[2] = std::move(other._v[2]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator = (const LVecBase3f &fill) {
  _v[0] = fill[0];
  _v[1] = fill[1];
  _v[2] = fill[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
set(const FloatType &x, const FloatType &y, const FloatType &z) {
  _v[0] = x;
  _v[1] = y;
  _v[2] = z;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
fill(const LVecBase3f &fill) {
  _v[0] = fill[0];
  _v[1] = fill[1];
  _v[2] = fill[2];
}

/**
 * Returns the cross product of this vector with the other vector.
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
cross(const SIMDVector3 &other) const {
  SIMDVector3<FloatType> result;

  result._v[0] = _v[1] * other._v[2];
  result._v[0].msub_in_place(_v[2], other._v[1]);

  result._v[1] = _v[2] * other._v[0];
  result._v[1].msub_in_place(_v[0], other._v[2]);

  result._v[2] = _v[0] * other._v[1];
  result._v[2].msub_in_place(_v[1], other._v[0]);

  return result;
}

/**
 * Returns the dot product of this vector with the other vector.
 */
template<class FloatType>
ALWAYS_INLINE FloatType SIMDVector3<FloatType>::
dot(const SIMDVector3 &other) const {
  FloatType dot = _v[0] * other._v[0];
  dot.madd_in_place(_v[1], other._v[1]);
  dot.madd_in_place(_v[2], other._v[2]);
  return dot;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
componentwise_mult(const SIMDVector3 &other) {
  _v[0] *= other._v[0];
  _v[1] *= other._v[1];
  _v[2] *= other._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
madd(const SIMDVector3 &m1, const FloatType &m2) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0].madd(m1._v[0], m2);
  result._v[1] = _v[1].madd(m1._v[1], m2);
  result._v[2] = _v[2].madd(m1._v[2], m2);
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
madd_in_place(const SIMDVector3 &m1, const FloatType &m2) {
  operator = (madd(m1, m2));
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
msub(const SIMDVector3 &m1, const FloatType &m2) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0].msub(m1._v[0], m2);
  result._v[1] = _v[1].msub(m1._v[1], m2);
  result._v[2] = _v[2].msub(m1._v[2], m2);
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
msub_in_place(const SIMDVector3 &m1, const FloatType &m2) {
  operator = (msub(m1, m2));
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator + (const SIMDVector3 &other) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] + other._v[0];
  result._v[1] = _v[1] + other._v[1];
  result._v[2] = _v[2] + other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator - (const SIMDVector3 &other) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] - other._v[0];
  result._v[1] = _v[1] - other._v[1];
  result._v[2] = _v[2] - other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator * (const SIMDVector3 &other) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] * other._v[0];
  result._v[1] = _v[1] * other._v[1];
  result._v[2] = _v[2] * other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator / (const SIMDVector3 &other) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] / other._v[0];
  result._v[1] = _v[1] / other._v[1];
  result._v[2] = _v[2] / other._v[2];
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator * (const FloatType &scalar) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] * scalar;
  result._v[1] = _v[1] * scalar;
  result._v[2] = _v[2] * scalar;
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator / (const FloatType &scalar) const {
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] / scalar;
  result._v[1] = _v[1] / scalar;
  result._v[2] = _v[2] / scalar;
  return result;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
operator - () const {
  SIMDVector3<FloatType> result;
  result._v[0] = -_v[0];
  result._v[1] = -_v[1];
  result._v[2] = -_v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator += (const SIMDVector3 &other) {
  _v[0] += other._v[0];
  _v[1] += other._v[1];
  _v[2] += other._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator -= (const SIMDVector3 &other) {
  _v[0] -= other._v[0];
  _v[1] -= other._v[1];
  _v[2] -= other._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator *= (const SIMDVector3 &other) {
  _v[0] *= other._v[0];
  _v[1] *= other._v[1];
  _v[2] *= other._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator /= (const SIMDVector3 &other) {
  _v[0] /= other._v[0];
  _v[1] /= other._v[1];
  _v[2] /= other._v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator *= (const FloatType &scalar) {
  _v[0] *= scalar;
  _v[1] *= scalar;
  _v[2] *= scalar;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
operator /= (const FloatType &scalar) {
  _v[0] /= scalar;
  _v[1] /= scalar;
  _v[2] /= scalar;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE FloatType SIMDVector3<FloatType>::
length() const {
  return simd_sqrt(dot(*this));
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE FloatType SIMDVector3<FloatType>::
length_squared() const {
  return dot(*this);
}

/**
 * Normalizes the vector (or vectors in this case) in place.
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
normalize() {
  FloatType len_recip = simd_rsqrt(length_squared());
  _v[0] *= len_recip;
  _v[1] *= len_recip;
  _v[2] *= len_recip;
}

/**
 * Returns a normalized version of this vector.
 */
template<class FloatType>
ALWAYS_INLINE SIMDVector3<FloatType> SIMDVector3<FloatType>::
normalized() const {
  FloatType len_recip = simd_rsqrt(length_squared());
  SIMDVector3<FloatType> result;
  result._v[0] = _v[0] * len_recip;
  result._v[1] = _v[1] * len_recip;
  result._v[2] = _v[2] * len_recip;
  return result;
}

/**
 * Returns the X component of the vector.
 */
template<class FloatType>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType>::
get_x() const {
  return _v[0];
}

/**
 * Returns the X component of the vector.
 */
template<class FloatType>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType>::
get_x() {
  return _v[0];
}

/**
 * Returns the Y component of the vector.
 */
template<class FloatType>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType>::
get_y() const {
  return _v[1];
}

/**
 * Returns the Y component of the vector.
 */
template<class FloatType>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType>::
get_y() {
  return _v[1];
}

/**
 * Returns the Z component of the vector.
 */
template<class FloatType>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType>::
get_z() const {
  return _v[2];
}

/**
 * Returns the Z component of the vector.
 */
template<class FloatType>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType>::
get_z() {
  return _v[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE LVecBase3f SIMDVector3<FloatType>::
get_lvec(int n) const {
  return LVecBase3f(_v[0][n], _v[1][n], _v[2][n]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
get_lvec(int n, LVecBase3f &vec) const {
  vec.set(_v[0][n], _v[1][n], _v[2][n]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDVector3<FloatType>::
set_lvec(int n, const LVecBase3f &vec) {
  _v[0][n] = vec[0];
  _v[1][n] = vec[1];
  _v[2][n] = vec[2];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE FloatType &SIMDVector3<FloatType>::
operator [] (int n) {
  return _v[n];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE const FloatType &SIMDVector3<FloatType>::
operator [] (int n) const {
  return _v[n];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE constexpr int SIMDVector3<FloatType>::
get_num_vectors() {
  return num_vectors;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE std::ostream &SIMDVector3<FloatType>::
output(std::ostream &out) const {
  out << "SIMDVector3 (" << num_vectors << " vectors)\n";
  out << "\tx ";
  _v[0].output(out) << "\n";
  out << "\ty ";
  _v[1].output(out) << "\n";
  out << "\tz ";
  _v[2].output(out) << "\n";
  return out;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType>::
SIMDQuaternion(const FloatType &r, const FloatType &i, const FloatType &j, const FloatType &k) {
  _v[0] = r;
  _v[1] = i;
  _v[2] = j;
  _v[3] = k;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType>::
SIMDQuaternion(const LQuaternionf &fill) {
  _v[0] = fill[0];
  _v[1] = fill[1];
  _v[2] = fill[2];
  _v[3] = fill[3];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType>::
SIMDQuaternion(const SIMDQuaternion &copy) {
  _v[0] = copy._v[0];
  _v[1] = copy._v[1];
  _v[2] = copy._v[2];
  _v[3] = copy._v[3];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType>::
SIMDQuaternion(SIMDQuaternion &&other) {
  _v[0] = std::move(other._v[0]);
  _v[1] = std::move(other._v[1]);
  _v[2] = std::move(other._v[2]);
  _v[3] = std::move(other._v[3]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
operator = (const LQuaternionf &fill) {
  _v[0] = fill[0];
  _v[1] = fill[1];
  _v[2] = fill[2];
  _v[3] = fill[3];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
operator = (const SIMDQuaternion &copy) {
  _v[0] = copy._v[0];
  _v[1] = copy._v[1];
  _v[2] = copy._v[2];
  _v[3] = copy._v[3];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
operator = (SIMDQuaternion &&other) {
  _v[0] = std::move(other._v[0]);
  _v[1] = std::move(other._v[1]);
  _v[2] = std::move(other._v[2]);
  _v[3] = std::move(other._v[3]);
}

/**
 * Concatenates the rotation of this quaternion with the other one, and
 * returns the result.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
operator * (const SIMDQuaternion &other) const {
  SIMDQuaternion<FloatType> ret;

  FloatType dot = other._v[0] * _v[0];

  ret._v[0] = dot;
  ret._v[1] = other._v[1] * _v[0];
  ret._v[2] = other._v[2] * _v[0];
  ret._v[3] = other._v[3] * _v[0];

  dot.madd_in_place(other._v[1], _v[1]);
  dot.madd_in_place(other._v[2], _v[2]);
  dot.madd_in_place(other._v[3], _v[3]);

  ret._v[0].msub_in_place(other._v[1], _v[1]);
  ret._v[0].msub_in_place(other._v[2], _v[2]);
  ret._v[0].msub_in_place(other._v[3], _v[3]);
  ret._v[1].madd_in_place(other._v[0], _v[1]);
  ret._v[1].msub_in_place(other._v[3], _v[2]);
  ret._v[1].madd_in_place(other._v[2], _v[3]);
  ret._v[2].madd_in_place(other._v[3], _v[1]);
  ret._v[2].madd_in_place(other._v[0], _v[2]);
  ret._v[2].msub_in_place(other._v[1], _v[3]);
  ret._v[3].msub_in_place(other._v[2], _v[1]);
  ret._v[3].madd_in_place(other._v[1], _v[2]);
  ret._v[3].madd_in_place(other._v[0], _v[3]);

  FloatType mask = dot < 0;
  ret._v[0] = FloatType::blend(ret._v[0], -ret._v[0], mask);
  ret._v[1] = FloatType::blend(ret._v[1], -ret._v[1], mask);
  ret._v[2] = FloatType::blend(ret._v[2], -ret._v[2], mask);
  ret._v[3] = FloatType::blend(ret._v[3], -ret._v[3], mask);

  return ret;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
operator *= (const SIMDQuaternion &other) {
  operator = (operator * (other));
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
operator - () const {
  SIMDQuaternion<FloatType> ret;
  ret._v[0] = -_v[0];
  ret._v[1] = -_v[1];
  ret._v[2] = -_v[2];
  ret._v[3] = -_v[3];
  return ret;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE FloatType SIMDQuaternion<FloatType>::
dot(const SIMDQuaternion &other) const {
  FloatType dot = _v[0] * other._v[0];
  dot.madd_in_place(_v[1], other._v[1]);
  dot.madd_in_place(_v[2], other._v[2]);
  dot.madd_in_place(_v[3], other._v[3]);
  return dot;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
scale_angle(const FloatType &scale) const {
  FloatType v1 = 1.0f;
  FloatType v0 = 0.0f;
  FloatType vflt_epsilon = FLT_EPSILON;

  FloatType sinom = _v[1] * _v[1];
  sinom.madd_in_place(_v[2], _v[2]);
  sinom.madd_in_place(_v[3], _v[3]);
  sinom = simd_min(sinom, v1);

  FloatType sinsom = simd_asin(sinom);
  sinsom *= scale;
  sinsom = simd_sin(sinsom);

  FloatType t = sinsom;
  t *= simd_recip(sinom + vflt_epsilon);

  SIMDQuaternion<FloatType> ret;

  ret._v[1] = _v[1] * t;
  ret._v[2] = _v[2] * t;
  ret._v[3] = _v[3] * t;

  FloatType r = v1 - sinsom * sinsom;
  r = simd_max(r, v0);
  r = simd_sqrt(r);

  // Keep sign of rotation.
  ret._v[0] = FloatType::blend(r, -r, (_v[0] < v0));

  return ret;
}

/**
 * Accumulates the other quaternion onto this quaternion and
 * returns the result.
 *
 * This is a concatenation of the rotations.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
accumulate(const SIMDQuaternion &other) const {
  return operator * (other);
}

/**
 * Accumulates the other quaternion onto this quaternion and
 * returns the result.
 *
 * This is a concatenation of the rotations.
 *
 * Uses the Source Engine convention for quaternion accumulation.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
accumulate_source(const SIMDQuaternion &other) const {
  SIMDQuaternion<FloatType> ret;

  FloatType dot = _v[0] * other._v[0];
  ret._v[0] = dot;
  ret._v[1] = _v[0] * other._v[1];
  ret._v[2] = _v[0] * other._v[2];
  ret._v[3] = _v[0] * other._v[3];

  dot.madd_in_place(_v[1], other._v[1]);
  ret._v[0].msub_in_place(_v[1], other._v[1]);
  ret._v[1].madd_in_place(_v[1], other._v[0]);
  ret._v[2].msub_in_place(_v[1], other._v[3]);
  ret._v[3].madd_in_place(_v[1], other._v[2]);

  dot.madd_in_place(_v[2], other._v[2]);
  ret._v[0].msub_in_place(_v[2], other._v[2]);
  ret._v[1].madd_in_place(_v[2], other._v[3]);
  ret._v[2].madd_in_place(_v[2], other._v[0]);
  ret._v[3].msub_in_place(_v[2], other._v[1]);

  dot.madd_in_place(_v[3], other._v[3]);
  ret._v[0].msub_in_place(_v[3], other._v[3]);
  ret._v[1].msub_in_place(_v[3], other._v[2]);
  ret._v[2].madd_in_place(_v[3], other._v[1]);
  ret._v[3].madd_in_place(_v[3], other._v[0]);

  FloatType mask = dot < 0;
  ret._v[0] = FloatType::blend(ret._v[0], -ret._v[0], mask);
  ret._v[1] = FloatType::blend(ret._v[1], -ret._v[1], mask);
  ret._v[2] = FloatType::blend(ret._v[2], -ret._v[2], mask);
  ret._v[3] = FloatType::blend(ret._v[3], -ret._v[3], mask);

  return ret;
}

/**
 * Accumulates a fraction of the other quaternion onto this quaternion
 * and returns the result.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
accumulate_scaled_rhs(const SIMDQuaternion &rhs, const FloatType &rhs_scale) const {
  return accumulate(rhs.scale_angle(rhs_scale));
}

/**
 * Accumulates a fraction of the other quaternion onto this quaternion
 * and returns the result.
 *
 * Uses the Source Engine convention for quaternion accumulation.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
accumulate_scaled_rhs_source(const SIMDQuaternion &rhs, const FloatType &rhs_scale) const {
  return accumulate_source(rhs.scale_angle(rhs_scale));
}

/**
 * Accumulates the other quaternion onto a fraction of this quaternion and
 * returns the result.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
accumulate_scaled_lhs(const SIMDQuaternion &rhs, const FloatType &lhs_scale) const {
  return scale_angle(lhs_scale).accumulate(rhs);
}

/**
 * Accumulates the other quaternion onto a fraction of this quaternion and
 * returns the result.
 *
 * Uses the Source Engine convention for quaternion accumulation.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
accumulate_scaled_lhs_source(const SIMDQuaternion &rhs, const FloatType &lhs_scale) const {
  return scale_angle(lhs_scale).accumulate_source(rhs);
}

/**
 * Returns this quaternion flipped if it is facing away from the other
 * quaternion.
 *
 * Needed to avoid jumps when interpolating between quaternions with
 * different signs.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
align(const SIMDQuaternion &other) const {
  FloatType mask = dot(other) < 0;
  SIMDQuaternion<FloatType> ret;
  ret._v[0] = FloatType::blend(_v[0], -_v[0], mask);
  ret._v[1] = FloatType::blend(_v[1], -_v[1], mask);
  ret._v[2] = FloatType::blend(_v[2], -_v[2], mask);
  ret._v[3] = FloatType::blend(_v[3], -_v[3], mask);
  return ret;
}

/**
 * Flips this quaternion in-place if it is facing away from the other
 * quaternion.
 *
 * Needed to avoid jumps when interpolating between quaternions with
 * different signs.
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
align_in_place(const SIMDQuaternion &other) {
  operator = (align(other));
}

/**
 * Normalizes all of the quaternions in the SIMDQuaternion, and returns the
 * result.
 *
 * Zero-length quats return the original quat.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
normalized() const {
  FloatType radius = dot(*this);
  FloatType is_zero_length = (radius == 0);
  FloatType inv_radius = simd_rsqrt(radius);
  SIMDQuaternion<FloatType> ret;
  // Return original quat where length was 0.
  ret._v[0] = FloatType::blend(_v[0] * inv_radius, _v[0], is_zero_length);
  ret._v[1] = FloatType::blend(_v[1] * inv_radius, _v[1], is_zero_length);
  ret._v[2] = FloatType::blend(_v[2] * inv_radius, _v[2], is_zero_length);
  ret._v[3] = FloatType::blend(_v[3] * inv_radius, _v[3], is_zero_length);
  return ret;
}


/**
 * Normalizes all of the quaternions in-place.
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
normalize() {
  operator = (normalized());
}

/**
 * Linearly interpolates between this quat and the other quat using the
 * given 0-1 fraction.  0 returns this quat, 1 returns the other quat.
 *
 * Cheaper than slerp(), but not as correct of an interpolation between
 * quaternion rotations.  Angular velocity is non-uniform as the rotations
 * move between each other.
 * https://tiborstanko.sk/lerp-vs-slerp.html
 *
 * Does not align the quats before interpolating.  See align_lerp().
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
lerp(const SIMDQuaternion &other, const FloatType &frac) const {
  FloatType e0 = FloatType(1.0f) - frac;

  SIMDQuaternion<FloatType> ret;

  ret._v[0] = _v[0] * e0;
  ret._v[0].madd_in_place(other._v[0], frac);
  ret._v[1] = _v[1] * e0;
  ret._v[1].madd_in_place(other._v[1], frac);
  ret._v[2] = _v[2] * e0;
  ret._v[2].madd_in_place(other._v[2], frac);
  ret._v[3] = _v[3] * e0;
  ret._v[3].madd_in_place(other._v[3], frac);

  return ret.normalized();
}

/**
 * Linearly interpolates between this quat and the other quat.  Ensures the
 * two quats are aligned before interpolating.
 *
 * See lerp() for more information on linear interpolation between quats.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
align_lerp(const SIMDQuaternion &other, const FloatType &frac) const {
  return align(other).lerp(other, frac);
}

/**
 * Performs spherical interpolation between this quat and the other quat
 * using the given 0-1 fraction.  0 returns this quat, 1 returns the other
 * quat.
 *
 * In comparision to lerp(), this is the correct way to interpolate between
 * quaternion rotations, but is more expensive.  Angular velocity remains
 * uniform as the rotations move between each other.
 * https://tiborstanko.sk/lerp-vs-slerp.html
 *
 * Does not align the quats before interpolating.  See align_slerp().
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
slerp(const SIMDQuaternion &other, const FloatType &frac) const {
  FloatType e0 = FloatType(1.0f) - frac;

  SIMDQuaternion<FloatType> ret;

  FloatType cos_omega;
  cos_omega = _v[0] * other._v[0];
  cos_omega.madd_in_place(_v[1], other._v[1]);
  cos_omega.madd_in_place(_v[2], other._v[2]);
  cos_omega.madd_in_place(_v[3], other._v[3]);

  FloatType sin_omega = 1.0f;

  FloatType cos_omega_lt_one = (cos_omega < (1.0f - 0.000001f));

  sin_omega.msub_in_place(cos_omega, cos_omega);
  FloatType inv_sin_omega = simd_rsqrt(sin_omega);
  sin_omega *= inv_sin_omega;

  FloatType omega = simd_atan2(sin_omega, cos_omega);

  FloatType alpha = omega * e0;
  FloatType beta = omega * frac;
  alpha = simd_sin(alpha);
  beta = simd_sin(beta);
  alpha *= inv_sin_omega;
  beta *= inv_sin_omega;

  // Fall back to linear interpolation if the quats aren't aligned.
  alpha = FloatType::blend(e0, alpha, cos_omega_lt_one);
  beta = FloatType::blend(frac, beta, cos_omega_lt_one);

  ret._v[0] = _v[0] * alpha;
  ret._v[1] = _v[1] * alpha;
  ret._v[2] = _v[2] * alpha;
  ret._v[3] = _v[3] * alpha;

  ret._v[0].madd_in_place(other._v[0], beta);
  ret._v[1].madd_in_place(other._v[1], beta);
  ret._v[2].madd_in_place(other._v[2], beta);
  ret._v[3].madd_in_place(other._v[3], beta);

  return ret;
}

/**
 * Spherically interpolates between this quat and the other quat.  Ensures
 * the two quats are aligned before interpolating.
 *
 * See slerp() for more information on spherical interpolation between quats.
 */
template<class FloatType>
ALWAYS_INLINE SIMDQuaternion<FloatType> SIMDQuaternion<FloatType>::
align_slerp(const SIMDQuaternion &other, const FloatType &frac) const {
  return align(other).slerp(other, frac);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE LQuaternionf SIMDQuaternion<FloatType>::
get_lquat(int n) const {
  return LQuaternionf(_v[0][n], _v[1][n], _v[2][n], _v[3][n]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
get_lquat(int n, LQuaternionf &quat) const {
  quat.set(_v[0][n], _v[1][n], _v[2][n], _v[3][n]);
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE void SIMDQuaternion<FloatType>::
set_lquat(int n, const LQuaternionf &quat) {
  _v[0][n] = quat[0];
  _v[1][n] = quat[1];
  _v[2][n] = quat[2];
  _v[3][n] = quat[3];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE FloatType &SIMDQuaternion<FloatType>::
operator [] (int n) {
  return _v[n];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE const FloatType &SIMDQuaternion<FloatType>::
operator [] (int n) const {
  return _v[n];
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE constexpr int SIMDQuaternion<FloatType>::
get_num_quats() {
  return num_quats;
}

/**
 *
 */
template<class FloatType>
ALWAYS_INLINE std::ostream &SIMDQuaternion<FloatType>::
output(std::ostream &out) const {
  out << "SIMDQuaternion (" << num_quats << " quats)\n";
  out << "\tr ";
  _v[0].output(out) << "\n";
  out << "\ti ";
  _v[1].output(out) << "\n";
  out << "\tj ";
  _v[2].output(out) << "\n";
  out << "\tk ";
  _v[3].output(out) << "\n";
  return out;
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
