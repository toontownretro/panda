/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramAttrib.cxx
 * @author lachbr
 * @date 2020-10-15
 */

#include "paramAttrib.h"
#include "bamReader.h"

TypeHandle ParamAttrib::_type_handle;
int ParamAttrib::_attrib_slot;

/**
 *
 */
void ParamAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  for (size_t i = 0; i < _params.size(); i++) {
    out << " ";
    out << _params.get_key(i) << ":" << _params.get_data(i);
  }
}

/**
 * Intended to be overridden by derived ShaderAttrib types to return a unique
 * number indicating whether this ShaderAttrib is equivalent to the other one.
 *
 * This should return 0 if the two ShaderAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ShaderAttrib objects whose get_type()
 * functions return the same.
 */
int ParamAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ParamAttrib *that = (const ParamAttrib *)other;

  if (_params.size() != that->_params.size()) {
    return (_params.size() < that->_params.size()) ? -1 : 1;
  }

  for (size_t i = 0; i < _params.size(); i++) {
    const std::string &mykey = _params.get_key(i);
    const std::string &myval = _params.get_data(i);

    const std::string &thatkey = that->_params.get_key(i);
    const std::string &thatval = that->_params.get_data(i);

    int key_cmp = mykey.compare(thatkey);
    if (key_cmp != 0) {
      return key_cmp;
    }

    int val_cmp = myval.compare(thatval);
    if (val_cmp != 0) {
      return val_cmp;
    }
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t ParamAttrib::
get_hash_impl() const {
  size_t hash = 0;

  hash = size_t_hash::add_hash(hash, _params.size());
  for (size_t i = 0; i < _params.size(); i++) {
    hash = string_hash::add_hash(hash, _params.get_key(i));
    hash = string_hash::add_hash(hash, _params.get_data(i));
  }

  return hash;
}

/**
 *
 */
CPT(RenderAttrib) ParamAttrib::
compose_impl(const RenderAttrib *other) const {
  ParamAttrib *attr = new ParamAttrib(*this);
  const ParamAttrib *over = (const ParamAttrib *)other;

  for (size_t i = 0; i < over->_params.size(); i++) {
    attr->_params[over->_params.get_key(i)] = over->_params.get_data(i);
  }

  return return_new(attr);
}

/**
 * Tells the BamReader how to create objects of type ParamAttrib.
 */
void ParamAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ParamAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint32(_params.size());
  for (size_t i = 0; i < _params.size(); i++) {
    dg.add_string(_params.get_key(i));
    dg.add_string(_params.get_data(i));
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ParamAttrib is encountered in the Bam file.  It should create
 * the ParamAttrib and extract its information from the file.
 */
TypedWritable *ParamAttrib::
make_from_bam(const FactoryParams &params) {
  ParamAttrib *attrib = new ParamAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ParamAttrib.
 */
void ParamAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  size_t size = scan.get_uint32();
  for (size_t i = 0; i < size; i++) {
    std::string key = scan.get_string();
    std::string value = scan.get_string();
    _params.store(key, value);
  }
}
