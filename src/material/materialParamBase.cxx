/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamBase.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamBase.h"
#include "bamWriter.h"
#include "bamReader.h"

TypeHandle MaterialParamBase::_type_handle;

/**
 *
 */
void MaterialParamBase::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_string(get_name());
}

/**
 *
 */
void MaterialParamBase::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
}
