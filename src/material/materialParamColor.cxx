/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamColor.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamColor.h"
#include "pdxList.h"

TypeHandle MaterialParamColor::_type_handle;

/**
 *
 */
bool MaterialParamColor::
from_pdx(const PDXValue &val, const DSearchPath &search_path) {
  _value = LColor(255, 255, 255, 255);

  if (!val.to_vec4(_value)) {
    return false;
  }

  _value /= 255;

  return true;
}

/**
 *
 */
void MaterialParamColor::
to_pdx(PDXValue &val, const Filename &filename) {
  val.from_vec4(_value * 255);
}

/**
 *
 */
void MaterialParamColor::
write_datagram(BamWriter *manager, Datagram &dg) {
  MaterialParamBase::write_datagram(manager, dg);

  _value.write_datagram(dg);
}

/**
 *
 */
void MaterialParamColor::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void MaterialParamColor::
fillin(DatagramIterator &scan, BamReader *manager) {
  MaterialParamBase::fillin(scan, manager);

  _value.read_datagram(scan);
}

/**
 *
 */
TypedWritable *MaterialParamColor::
make_from_bam(const FactoryParams &params) {
  MaterialParamColor *param = new MaterialParamColor("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);
  return param;
}
