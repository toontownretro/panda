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
 *
 */
void PoseParameter::
write_datagram(BamWriter *manager, Datagram &dg) {
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
  _min = scan.get_stdfloat();
  _max = scan.get_stdfloat();
  _value = scan.get_stdfloat();
  _looping = scan.get_stdfloat();
}
