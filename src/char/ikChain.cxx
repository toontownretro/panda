/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikChain.cxx
 * @author lachbr
 * @date 2021-02-11
 */

#include "ikChain.h"
#include "partGroup.h"
#include "config_char.h"
#include "characterJoint.h"

TypeHandle IKChain::_type_handle;

/**
 *
 */
IKChain::
IKChain(const std::string &name, CharacterJoint *foot) :
  Namable(name) {

  _foot = foot;
  _knee = (CharacterJoint *)_foot->get_parent();
  _hip = (CharacterJoint *)_knee->get_parent();

  _height = 0.0;
  _floor = 0.0;
  _pad = 0.0;

  _enabled = false;
}

/**
 * Executes an IK pass on all joints in the chain.  Calculates and applies a
 * suitable transform to each joint given the current end effector transform
 * and current transform of each joint in the chain.
 */
bool IKChain::
solve_ik() {

}

/**
 * Factory method to generate an IKChain object
 */
void IKChain::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void IKChain::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  manager->write_pointer(me, _hip);
  manager->write_pointer(me, _knee);
  manager->write_pointer(me, _foot);
  _knee_direction.write_datagram(me);
  _center.write_datagram(me);
  me.add_stdfloat(_height);
  me.add_stdfloat(_floor);
  me.add_stdfloat(_pad);
}

/**
 * Takes in a vector of pointes to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int IKChain::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _hip = DCAST(CharacterJoint, p_list[pi++]);
  _knee = DCAST(CharacterJoint, p_list[pi++]);
  _foot = DCAST(CharacterJoint, p_list[pi++]);

  return pi;
}

/**
 * Factory method to generate a IKChain object
 */
TypedWritable *IKChain::
make_from_bam(const FactoryParams &params) {
  IKChain *me = new IKChain;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void IKChain::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  manager->read_pointers(scan, 3);
  _knee_direction.read_datagram(scan);
  _center.read_datagram(scan);
  _height = scan.get_stdfloat();
  _floor = scan.get_stdfloat();
  _pad = scan.get_stdfloat();
}
