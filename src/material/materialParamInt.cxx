/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamInt.cxx
 * @author brian
 * @date 2023-11-12
 */

#include "materialParamInt.h"

#include "string_utils.h"

TypeHandle MaterialParamInt::_type_handle;

/**
 *
 */
bool MaterialParamInt::
from_pdx(const PDXValue &val, const DSearchPath &search_path) {
  _value = val.get_int();
  return true;
}

/**
 *
 */
void MaterialParamInt::
to_pdx(PDXValue &val, const Filename &filename) {
  val.set_int(_value);
}

/**
 *
 */
void MaterialParamInt::
write_datagram(BamWriter *manager, Datagram &dg) {
  MaterialParamBase::write_datagram(manager, dg);
  dg.add_int32(_value);
}

/**
 *
 */
void MaterialParamInt::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void MaterialParamInt::
fillin(DatagramIterator &dgi, BamReader *manager) {
  MaterialParamBase::fillin(dgi, manager);
  _value = dgi.get_int32();
}

/**
 *
 */
TypedWritable *MaterialParamInt::
make_from_bam(const FactoryParams &params) {
  MaterialParamInt *param = new MaterialParamInt("");
  DatagramIterator dgi;
  BamReader *manager;
  parse_params(params, dgi, manager);
  param->fillin(dgi, manager);

  return param;
}
