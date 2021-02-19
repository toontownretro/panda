/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelRoot.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "modelRoot.h"

TypeHandle ModelRoot::_type_handle;

/**
 * Recursive implementation of set_active_material_group().
 */
void ModelRoot::
r_set_active_material_group(PandaNode *node, size_t n) {
  if (node != this && node->is_of_type(ModelRoot::get_class_type())) {
    // We reached another ModelRoot.  Presumably this is another model that we
    // should not try to muck with.
    return;
  }

  MaterialGroup *group = _material_groups[_active_material_group];
  MaterialGroup *new_group = _material_groups[n];

  for (size_t i = 0; i < group->get_num_materials(); i++) {
    node->replace_state(group->get_material(i), new_group->get_material(i));
  }

  for (int i = 0; i < node->get_num_children(); i++) {
    PandaNode *child = node->get_child(i);
    r_set_active_material_group(child, n);
  }
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *ModelRoot::
make_copy() const {
  return new ModelRoot(*this);
}

/**
 * Tells the BamReader how to create objects of type ModelRoot.
 */
void ModelRoot::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ModelRoot::
write_datagram(BamWriter *manager, Datagram &dg) {
  ModelNode::write_datagram(manager, dg);

  dg.add_uint8(_material_groups.size());
  for (size_t i = 0; i < _material_groups.size(); i++) {
    manager->write_pointer(dg, _material_groups[i]);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int ModelRoot::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ModelNode::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _material_groups.size(); i++) {
    TypedWritable *p = p_list[pi++];
    MaterialGroup *group;
    DCAST_INTO_R(group, p, pi);
    _material_groups[i] = group;
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ModelRoot is encountered in the Bam file.  It should create the
 * ModelRoot and extract its information from the file.
 */
TypedWritable *ModelRoot::
make_from_bam(const FactoryParams &params) {
  ModelRoot *node = new ModelRoot("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ModelRoot.
 */
void ModelRoot::
fillin(DatagramIterator &scan, BamReader *manager) {
  ModelNode::fillin(scan, manager);

  _material_groups.resize(scan.get_uint8());
  manager->read_pointers(scan, _material_groups.size());
}
