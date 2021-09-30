/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.cxx
 * @author lachbr
 * @date 2021-03-21
 */

#include "materialAttrib.h"
#include "virtualFileSystem.h"
#include "materialPool.h"
#include "config_pgraph.h"

TypeHandle MaterialAttrib::_type_handle;
int MaterialAttrib::_attrib_slot;

/**
 * Returns a new MaterialAttrib that contains no material.
 */
CPT(RenderAttrib) MaterialAttrib::
make_off() {
  MaterialAttrib *attr = new MaterialAttrib;
  attr->_is_off = true;

  return return_new(attr);
}

/**
 * Returns a new MaterialAttrib that references the indicated Material.
 */
CPT(RenderAttrib) MaterialAttrib::
make(Material *material) {
  MaterialAttrib *attr = new MaterialAttrib;
  attr->_material = material;

  return return_new(attr);
}

/**
 * Returns the default MaterialAttrib.
 */
CPT(RenderAttrib) MaterialAttrib::
make_default() {
  return RenderAttribRegistry::get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 *
 */
int MaterialAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const MaterialAttrib *that = (const MaterialAttrib *)other;

  if (_material != that->_material) {
    return _material < that->_material ? -1 : 1;
  }

  return 0;
}

/**
 *
 */
size_t MaterialAttrib::
get_hash_impl() const {
  return pointer_hash::add_hash(0, _material);
}

/**
 *
 */
void MaterialAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MaterialAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);
  dg.add_bool(_is_off);
  manager->write_pointer(dg, _material);
}

/**
 *
 */
TypedWritable *MaterialAttrib::
make_from_bam(const FactoryParams &params) {
  MaterialAttrib *attr = new MaterialAttrib;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  attr->fillin(scan, manager);
  return attr;
}

/**
 *
 */
void MaterialAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);
  _is_off = scan.get_bool();
  manager->read_pointer(scan);
}

/**
 *
 */
int MaterialAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  Material *mat;
  DCAST_INTO_R(mat, p_list[pi++], pi);
  _material = mat;

  return pi;
}
