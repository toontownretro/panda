/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointTransformTable.cxx
 * @author theclashingfritz
 * @date 2025-03-18
 */
 
#include "jointTransformTable.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "character.h"
#include "jointVertexTransform.h"

TypeHandle JointTransformTable::_type_handle;

/**
 * Constructs an invalid object; used only by the bam loader.
 */
JointTransformTable::
JointTransformTable() :
  TransformTable()
{
}

/**
 *
 */
JointTransformTable::
JointTransformTable(Character *character) :
  TransformTable(),
  _char(character)
{
}

/**
 *
 */
JointTransformTable::
JointTransformTable(const TransformTable &copy, Character *character) :
  TransformTable(copy),
  _char(character)
{
}

/**
 *
 */
JointTransformTable::
JointTransformTable(const JointTransformTable &copy) :
  TransformTable(copy),
  _char(copy._char)
{
}

/**
 *
 */
void JointTransformTable::
operator = (const JointTransformTable &copy) {
  TransformTable::operator=(copy);
  _char = copy._char;
}

/**
 *
 */
JointTransformTable::
~JointTransformTable() {
}

/**
 *
 */
LMatrix4f *JointTransformTable::
get_transform_matrices(size_t num_matrices, Thread *current_thread) const {
  nassertr(num_matrices <= get_num_transforms(), nullptr);
  
  // We'll create the reader here instead,
  // This will result in a much faster overall access to the skinned matrices
  // from a Character.
  Character::RenderCDReader cdata(_char->_render_cycler, current_thread);
  TypeHandle jointVtxClss = JointVertexTransform::get_class_type();
  
  LMatrix4f *matrices = (LMatrix4f *)alloca(num_matrices * sizeof(LMatrix4f));
  for (size_t i = 0; i < num_matrices; ++i) {
    const VertexTransform *transform = get_transform(i);
    if (transform->is_of_type(jointVtxClss)) {
      const JointVertexTransform *jointTransform = DCAST(JointVertexTransform, transform);
#ifdef STDFLOAT_DOUBLE
      matrices[i] = LCAST(float, cdata->_joint_skinning_matrices[jointTransform->get_joint()]);
#else
      matrices[i] = cdata->_joint_skinning_matrices[jointTransform->get_joint()];
#endif
    } else {
#ifdef STDFLOAT_DOUBLE
      matrices[i] = LCAST(float, transform->get_matrix(current_thread));
#else
      matrices[i] = transform->get_matrix(current_thread);
#endif
    }
  }
  
  return matrices;
}

/**
 *
 */
LVecBase4f *JointTransformTable::
get_transform_vectors(size_t num_vectors, Thread *current_thread) const {
  nassertr(num_vectors <= get_num_transforms(), nullptr);
  
  LVecBase4f *vectors = (LVecBase4f *)alloca(num_vectors * sizeof(LVecBase4f) * 3);
  for (size_t i = 0; i < num_vectors; ++i) {
    const VertexTransform *transform = get_transform(i);
#ifdef STDFLOAT_DOUBLE
    LMatrix4f matrix = LCAST(float, transform->get_matrix(current_thread));
#else
    const LMatrix4f &matrix = transform->get_matrix(current_thread);
#endif
    vectors[i * 3 + 0] = matrix.get_row(0);
    vectors[i * 3 + 1] = matrix.get_row(1);
    vectors[i * 3 + 2] = matrix.get_row(2);
  }
  
  return vectors;
}

/**
 *
 */
void JointTransformTable::
write(std::ostream &out) const {
  nassertv(_char != nullptr);
  out << _char->get_name() << " " << _joint_count << "\n";
  TransformTable::write(out);
}

/**
 * Tells the BamReader how to create objects of type JointTransformTable.
 */
void JointTransformTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void JointTransformTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  TransformTable::write_datagram(manager, dg);

  manager->write_pointer(dg, _char.p());
  dg.add_int32(_joint_count);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int JointTransformTable::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TransformTable::complete_pointers(p_list, manager);
  _char = DCAST(Character, p_list[pi++]);
  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type JointTransformTable is encountered in the Bam file.  It should create
 * the JointTransformTable and extract its information from the file.
 */
TypedWritable *JointTransformTable::
make_from_bam(const FactoryParams &params) {
  JointTransformTable *object = new JointTransformTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new JointTransformTable.
 */
void JointTransformTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  TransformTable::fillin(scan, manager);

  manager->read_pointer(scan);
  _joint_count = scan.get_int32();
}