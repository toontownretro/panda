/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderParamAttrib.cxx
 * @author lachbr
 * @date 2020-10-15
 */

#include "shaderParamAttrib.h"

TypeHandle ShaderParamAttrib::_type_handle;
int ShaderParamAttrib::_attrib_slot;

/**
 *
 */
void ShaderParamAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  out << get_shader_name();
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
int ShaderParamAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ShaderParamAttrib *that = (const ShaderParamAttrib *)other;

  int name_cmp = _shader_name.compare(that->_shader_name);
  if (name_cmp != 0) {
    return name_cmp;
  }
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
size_t ShaderParamAttrib::
get_hash_impl() const {
  size_t hash = 0;

  hash = string_hash::add_hash(hash, _shader_name);
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
CPT(RenderAttrib) ShaderParamAttrib::
compose_impl(const RenderAttrib *other) const {
  ShaderParamAttrib *attr = new ShaderParamAttrib(*this);
  const ShaderParamAttrib *over = (const ShaderParamAttrib *)other;

  attr->_shader_name = over->_shader_name;
  for (size_t i = 0; i < over->_params.size(); i++) {
    attr->_params[over->_params.get_key(i)] = over->_params.get_data(i);
  }

  return return_new(attr);
}
