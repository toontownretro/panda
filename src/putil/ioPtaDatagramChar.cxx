/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioPtaDatagramChar.cxx
 * @author jason
 * @date 2000-06-26
 */

#include "pandabase.h"

#include "ioPtaDatagramChar.h"
#include "datagram.h"
#include "datagramIterator.h"

/**
 *
 */
void IoPtaDatagramChar::
write_datagram(BamWriter *, Datagram &dest, CPTA_uchar array) {
  dest.add_uint32(array.size());
  for(int i = 0; i < (int)array.size(); ++i) {
    dest.add_uint8(array[i]);
  }
}

/**
 *
 */
PTA_uchar IoPtaDatagramChar::
read_datagram(BamReader *, DatagramIterator &source) {
  PTA_uchar array;

  int size = source.get_uint32();
  for(int i = 0; i < size; ++i) {
    array.push_back(source.get_uint8());
  }

  return array;
}
