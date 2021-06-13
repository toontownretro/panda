/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdxList.I
 * @author brian
 * @date 2021-06-10
 */

/**
 * Adds the indicated PDXValue to the end of the list.
 */
INLINE void PDXList::
append(const PDXValue &value) {
  _values.push_back(value);
}

/**
 * Adds the indicated PDXValue to the beginning of the list.
 */
INLINE void PDXList::
prepend(const PDXValue &value) {
  _values.insert(_values.begin(), value);
}

/**
 * Inserts a PDXValue into the list at the indicated index.
 */
INLINE void PDXList::
insert(size_t n, const PDXValue &value) {
  _values.insert(_values.begin() + n, value);
}

/**
 * Removes the PDXValue from the list at the indicated index.
 */
INLINE void PDXList::
remove(size_t n) {
  nassertv(n < _values.size());
  _values.erase(_values.begin() + n);
}

/**
 * Ensures there is enough memory in the list to hold the indicated number of
 * PDXValues.
 */
INLINE void PDXList::
reserve(size_t count) {
  _values.reserve(count);
}

/**
 * Resizes the list to contain the indicated number of PDXValues.
 */
INLINE void PDXList::
resize(size_t count) {
  _values.resize(count);
}

/**
 * Returns the number of PDXValues in the list.
 */
INLINE size_t PDXList::
size() const {
  return _values.size();
}

/**
 * Returns the PDXValue at the indicated index.
 */
INLINE const PDXValue &PDXList::
get(size_t n) const {
  assert(n < _values.size());
  return _values[n];
}

/**
 * Returns the PDXValue at the indicated index.
 */
INLINE PDXValue &PDXList::
get(size_t n) {
  assert(n < _values.size());
  return _values[n];
}

/**
 *
 */
INLINE const PDXValue &PDXList::
operator [] (size_t n) const {
  return get(n);
}

/**
 *
 */
INLINE PDXValue &PDXList::
operator [] (size_t n) {
  return get(n);
}

/**
 * Returns the underlying vector of PDXValues.
 */
INLINE const PDXList::Values &PDXList::
get_values() const {
  return _values;
}

/**
 * Returns the underlying vector of PDXValues.
 */
INLINE PDXList::Values &PDXList::
get_values() {
  return _values;
}
