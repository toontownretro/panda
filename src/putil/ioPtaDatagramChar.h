/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ioPtaDatagramChar.h
 * @author brian
 * @date 2021-09-14
 */

#ifndef _IO_PTA_DATAGRAM_CHAR
#define _IO_PTA_DATAGRAM_CHAR

#include "pandabase.h"

#include "pointerToArray.h"
#include "pta_uchar.h"

class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;

/**
 * This class is used to read and write a PTA_uchar from a Datagram, in
 * support of Bam.  It's not intended to be constructed; it's just a
 * convenient place to scope these static methods which should be called
 * directly.
 */
class EXPCL_PANDA_PUTIL IoPtaDatagramChar {
public:
  static void write_datagram(BamWriter *manager, Datagram &dest, CPTA_uchar array);
  static PTA_uchar read_datagram(BamReader *manager, DatagramIterator &source);
};

typedef IoPtaDatagramChar IPD_uchar;

#endif
