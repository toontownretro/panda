/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialGroup.cxx
 * @author lachbr
 * @date 2021-02-14
 */

#include "materialGroup.h"
#include "bamReader.h"

TypeHandle MaterialGroup::_type_handle;

/**
 * Tells the BamReader how to create objects of type MaterialGroup.
 */
void MaterialGroup::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MaterialGroup::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  dg.add_uint8(get_num_materials());
  for (size_t i = 0; i < get_num_materials(); i++) {
    manager->write_pointer(dg, get_material(i));
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int MaterialGroup::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _states.size(); i++) {
    TypedWritable *p = p_list[pi++];
    const RenderState *state;
    DCAST_INTO_R(state, p, pi);
    _states[i] = state;
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type MaterialGroup is encountered in the Bam file.  It should create
 * the MaterialGroup and extract its information from the file.
 */
TypedWritable *MaterialGroup::
make_from_bam(const FactoryParams &params) {
  MaterialGroup *attrib = new MaterialGroup;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MaterialGroup.
 */
void MaterialGroup::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  _states.resize(scan.get_uint8());
  for (size_t i = 0; i < _states.size(); i++) {
    manager->read_pointer(scan);
  }
}
