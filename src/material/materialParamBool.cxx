/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamBool.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamBool.h"

TypeHandle MaterialParamBool::_type_handle;

/**
 *
 */
bool MaterialParamBool::
from_pdx(const PDXValue &val, const DSearchPath &search_path) {
  _value = val.get_bool();
  return true;
}

/**
 *
 */
void MaterialParamBool::
to_pdx(PDXValue &val, const Filename &filename) {
  val.set_bool(_value);
}

/**
 *
 */
void MaterialParamBool::
write_datagram(BamWriter *manager, Datagram &dg) {
  MaterialParamBase::write_datagram(manager, dg);

  dg.add_bool(_value);
}

/**
 *
 */
void MaterialParamBool::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void MaterialParamBool::
fillin(DatagramIterator &scan, BamReader *manager) {
  MaterialParamBase::fillin(scan, manager);

  _value = scan.get_bool();
}

/**
 *
 */
TypedWritable *MaterialParamBool::
make_from_bam(const FactoryParams &params) {
  MaterialParamBool *param = new MaterialParamBool("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);
  return param;
}
