/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userVertexTransform.cxx
 * @author drose
 * @date 2005-03-24
 */

#include "userVertexTransform.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle UserVertexTransform::_type_handle;

/**
 *
 */
UserVertexTransform::
UserVertexTransform(const std::string &name) :
  _name(name)
{
}

/**
 *
 */
void UserVertexTransform::
output(std::ostream &out) const {
  out << get_type() << " " << get_name();
}

/**
 * Tells the BamReader how to create objects of type UserVertexTransform.
 */
void UserVertexTransform::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void UserVertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexTransform::write_datagram(manager, dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type UserVertexTransform is encountered in the Bam file.  It should create
 * the UserVertexTransform and extract its information from the file.
 */
TypedWritable *UserVertexTransform::
make_from_bam(const FactoryParams &params) {
  UserVertexTransform *object = new UserVertexTransform("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new UserVertexTransform.
 */
void UserVertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexTransform::fillin(scan, manager);
}
