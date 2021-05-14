/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file poseParameter.I
 * @author brian
 * @date 2021-05-07
 */

/**
 *
 */
INLINE PoseParameter::
PoseParameter(const std::string &name, PN_stdfloat min, PN_stdfloat max) :
  Namable(name),
  _min(min),
  _max(max),
  _value(0.0f)
{
}

/**
 *
 */
INLINE void PoseParameter::
set_min(PN_stdfloat min) {
  _min = min;
  set_value(_value);
}

/**
 *
 */
INLINE PN_stdfloat PoseParameter::
get_min() const {
  return _min;
}

/**
 *
 */
INLINE void PoseParameter::
set_max(PN_stdfloat max) {
  _max = max;
  set_value(_value);
}

/**
 *
 */
INLINE PN_stdfloat PoseParameter::
get_max() const {
  return _max;
}

/**
 *
 */
INLINE void PoseParameter::
set_value(PN_stdfloat value) {
  _value = std::max(_min, std::min(_max, value));
}

/**
 *
 */
INLINE PN_stdfloat PoseParameter::
get_value() const {
  return _value;
}
