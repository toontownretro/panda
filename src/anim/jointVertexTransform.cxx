/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointVertexTransform.cxx
 * @author drose
 * @date 2005-03-24
 */

#include "jointVertexTransform.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "lightMutexHolder.h"
#include "characterJoint.h"
#include "character.h"

TypeHandle JointVertexTransform::_type_handle;

/**
 * Constructs an invalid object; used only by the bam loader.
 */
JointVertexTransform::
JointVertexTransform()
{
}

/**
 * Constructs a new object that converts vertices from the indicated joint's
 * coordinate space, into the other indicated joint's space.
 */
JointVertexTransform::
JointVertexTransform(Character *character, int joint) :
  _char(character),
  _joint(joint)
{
  // Tell the joint that we need to be informed when it moves.
  _char->set_joint_vertex_transform(this, joint);
  mark_modified(Thread::get_current_thread());
}

/**
 *
 */
JointVertexTransform::
~JointVertexTransform() {
  // Tell the joint to stop informing us about its motion.
  PT(Character) c = _char.lock();
  if (c != nullptr) {
    c->set_joint_vertex_transform(nullptr, _joint);
  }
}

/**
 *
 */
LMatrix4 JointVertexTransform::
get_matrix(Thread *current_thread) const {
  nassertr(_char != nullptr, LMatrix4::ident_mat());
  return _char->get_joint_skinning_matrix(_joint, current_thread);
}

/**
 *
 */
void JointVertexTransform::
output(std::ostream &out) const {
  nassertv(_char != nullptr);
  out << _char << " " << _char->get_joint_name(_joint) << " " << _joint;
}

/**
 * Tells the BamReader how to create objects of type JointVertexTransform.
 */
void JointVertexTransform::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void JointVertexTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexTransform::write_datagram(manager, dg);

  manager->write_pointer(dg, _char.p());
  dg.add_int16(_joint);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int JointVertexTransform::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = VertexTransform::complete_pointers(p_list, manager);

  _char = DCAST(Character, p_list[pi++]);
  _char->set_joint_vertex_transform(this, _joint);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type JointVertexTransform is encountered in the Bam file.  It should create
 * the JointVertexTransform and extract its information from the file.
 */
TypedWritable *JointVertexTransform::
make_from_bam(const FactoryParams &params) {
  JointVertexTransform *object = new JointVertexTransform;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new JointVertexTransform.
 */
void JointVertexTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexTransform::fillin(scan, manager);

  manager->read_pointer(scan);
  _joint = scan.get_int16();
  mark_modified(Thread::get_current_thread());
}
