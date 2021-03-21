/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialParamFloat.cxx
 * @author lachbr
 * @date 2021-03-07
 */

#include "materialParamFloat.h"
#include "string_utils.h"

TypeHandle MaterialParamFloat::_type_handle;

/**
 *
 */
bool MaterialParamFloat::
from_string(const std::string &str, const DSearchPath &search_path) {
  return string_to_stdfloat(str, _value);
}

/**
 *
 */
void MaterialParamFloat::
to_string(std::string &str, const Filename &filename) {
  str = std::to_string(_value);
}

/**
 *
 */
void MaterialParamFloat::
write_datagram(BamWriter *manager, Datagram &dg) {
  MaterialParamBase::write_datagram(manager, dg);
  dg.add_stdfloat(_value);
}

/**
 *
 */
void MaterialParamFloat::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
void MaterialParamFloat::
fillin(DatagramIterator &dgi, BamReader *manager) {
  MaterialParamBase::fillin(dgi, manager);
  _value = dgi.get_stdfloat();
}

/**
 *
 */
TypedWritable *MaterialParamFloat::
make_from_bam(const FactoryParams &params) {
  MaterialParamFloat *param = new MaterialParamFloat("");
  DatagramIterator dgi;
  BamReader *manager;
  parse_params(params, dgi, manager);
  param->fillin(dgi, manager);

  return param;
}
