/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weightList.I
 * @author brian
 * @date 2021-05-07
 */

/**
 *
 */
INLINE WeightListDesc::
WeightListDesc(const std::string &name) :
  Namable(name)
{
}

/**
 *
 */
INLINE void WeightListDesc::
set_weight(const std::string &name, PN_stdfloat weight) {
  _weights[name] = weight;
}

/**
 *
 */
INLINE PN_stdfloat WeightListDesc::
get_weight(const std::string &name) const {
  Weights::const_iterator it = _weights.find(name);
  if (it == _weights.end()) {
    return 0.0f;
  }

  return (*it).second;
}

/**
 *
 */
INLINE bool WeightListDesc::
has_weight(const std::string &name) const {
  Weights::const_iterator it = _weights.find(name);
  return it != _weights.end();
}

/**
 *
 */
INLINE size_t WeightList::
get_num_weights() const {
  return _weights.size();
}

/**
 *
 */
INLINE PN_stdfloat WeightList::
get_weight(size_t n) const {
  return _weights[n];
}
