/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderObject.cxx
 * @author lachbr
 * @date 2020-12-22
 */

#include "shaderObject.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle ShaderObject::_type_handle;

/**
 * Calculates the total number of possible combinations for combo variable
 * values.
 */
void ShaderObject::
calc_total_combos() {
  _total_combos = 1;
  for (size_t i = 0; i < _combos.size(); i++) {
    Combo &combo = _combos[i];
    // The scale is used to calculate a permutation index from all the combo
    // values.
    combo.scale = _total_combos;
    _total_combos *= combo.max_val - combo.min_val + 1;
  }
}

/**
 * Tells the BamReader how to create objects of type ShaderObject.
 */
void ShaderObject::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ShaderObject::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  // Write the combos.
  dg.add_uint32(_combos.size());
  for (size_t i = 0; i < _combos.size(); i++) {
    const Combo &combo = _combos[i];
    dg.add_string(combo.name->get_name());
    dg.add_int8(combo.min_val);
    dg.add_int8(combo.max_val);
  }

  // Write the permutations.
  dg.add_uint32(_permutations.size());
  for (size_t i = 0; i < _permutations.size(); i++) {
    manager->write_pointer(dg, _permutations[i].get_read_pointer());
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int ShaderObject::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _permutations.size(); i++) {
    if (p_list[index] != nullptr) {
      ShaderModuleSpirV *mod;
      DCAST_INTO_R(mod, p_list[index], index);
      _permutations[i] = mod;
    }
    index++;
  }

  return index;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ShaderObject is encountered in the Bam file.  It should create the
 * ShaderObject and extract its information from the file.
 */
TypedWritable *ShaderObject::
make_from_bam(const FactoryParams &params) {
  ShaderObject *sho = new ShaderObject;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  sho->fillin(scan, manager);

  return sho;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ShaderObject.
 */
void ShaderObject::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  size_t num_combos = scan.get_uint32();
  for (size_t i = 0; i < num_combos; i++) {
    Combo combo;
    combo.name = InternalName::make(scan.get_string());
    combo.min_val = scan.get_int8();
    combo.max_val = scan.get_int8();
    _combos.push_back(combo);
  }

  size_t num_permutations = scan.get_uint32();
  _permutations.resize(num_permutations);
  for (size_t i = 0; i < num_permutations; i++) {
    manager->read_pointer(scan);
  }

  calc_total_combos();
}
