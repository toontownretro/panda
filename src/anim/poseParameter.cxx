/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file poseParameter.cxx
 * @author brian
 * @date 2021-05-07
 */

#include "poseParameter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle PoseParameter::_type_handle;

/**
 * Sets the normalized 0..1 value of the pose parameter.
 */
void PoseParameter::
set_norm_value(PN_stdfloat value) {
  _value = std::clamp(value, 0.0f, 1.0f);
}

/**
 * Returns the normalized 0..1 value of the pose parameter.
 */
PN_stdfloat PoseParameter::
get_norm_value() const {
  return _value;
}

/**
 * Sets the ranged value of the pose parameter.  Converts the value to a
 * normalized 0..1 value before storing.
 */
void PoseParameter::
set_value(PN_stdfloat value) {
  if (_looping) {
    PN_stdfloat wrap = ((_min + _max) / 2.0f) + (_looping / 2.0f);
    PN_stdfloat shift = _looping - wrap;
    value = value - (_looping * std::floor((value + shift) / _looping));
  }

  _value = (value - _min) / (_max - _min);
  _value = std::clamp(_value, 0.0f, 1.0f);
}

/**
 * Returns the ranged value of the pose parameter.
 */
PN_stdfloat PoseParameter::
get_value() const {
  return _value * (_max - _min) + _min;
}

/**
 *
 */
void PoseParameter::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_string(get_name());
  dg.add_stdfloat(_min);
  dg.add_stdfloat(_max);
  dg.add_stdfloat(_value);
  dg.add_stdfloat(_looping);
}

/**
 *
 */
void PoseParameter::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  _min = scan.get_stdfloat();
  _max = scan.get_stdfloat();
  _value = scan.get_stdfloat();
  _looping = scan.get_stdfloat();
}
