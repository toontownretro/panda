/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamVector.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamVector.h"
#include "pdxList.h"

TypeHandle MaterialParamVector::_type_handle;

/**
 *
 */
bool MaterialParamVector::
from_pdx(const PDXValue &val, const DSearchPath &search_path) {
  _value = LVector3(1, 1, 1);

  if (!val.to_vec3(_value)) {
    return false;
  }

  return true;
}

/**
 *
 */
void MaterialParamVector::
to_pdx(PDXValue &val, const Filename &filename) {
  val.from_vec3(_value);
}

/**
 *
 */
void MaterialParamVector::
write_datagram(BamWriter *manager, Datagram &dg) {
  MaterialParamBase::write_datagram(manager, dg);

  _value.write_datagram(dg);
}

/**
 *
 */
void MaterialParamVector::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void MaterialParamVector::
fillin(DatagramIterator &scan, BamReader *manager) {
  MaterialParamBase::fillin(scan, manager);

  _value.read_datagram(scan);
}

/**
 *
 */
TypedWritable *MaterialParamVector::
make_from_bam(const FactoryParams &params) {
  MaterialParamVector *param = new MaterialParamVector("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);
  return param;
}
