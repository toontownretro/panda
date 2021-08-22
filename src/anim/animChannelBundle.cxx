/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBundle.cxx
 * @author brian
 * @date 2021-08-05
 */

#include "animChannelBundle.h"

IMPLEMENT_CLASS(AnimChannelBundle);

/**
 *
 */
PandaNode *AnimChannelBundle::
make_copy() const {
  return new AnimChannelBundle(*this);
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * Node by duplicating instances, false otherwise (for instance, a Camera
 * cannot be safely flattened, because the Camera pointer itself is
 * meaningful).
 */
bool AnimChannelBundle::
safe_to_flatten() const {
  return false;
}

/**
 * Tells the BamReader how to create objects of type AnimChannelBundle.
 */
void AnimChannelBundle::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AnimChannelBundle::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  dg.add_uint32(_channels.size());
  for (size_t i = 0; i < _channels.size(); i++) {
    manager->write_pointer(dg, _channels[i]);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int AnimChannelBundle::
complete_pointers(TypedWritable **p_list, BamReader* manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);
  for (size_t i = 0; i < _channels.size(); i++) {
    _channels[i] = DCAST(AnimChannel, p_list[pi++]);
  }
  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * this type is encountered in the Bam file.  It should create the object and
 * extract its information from the file.
 */
TypedWritable *AnimChannelBundle::
make_from_bam(const FactoryParams &params) {
  AnimChannelBundle *node = new AnimChannelBundle("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PandaNode.
 */
void AnimChannelBundle::
fillin(DatagramIterator &scan, BamReader* manager) {
  PandaNode::fillin(scan, manager);
  _channels.resize(scan.get_uint32());
  for (size_t i = 0; i < _channels.size(); i++) {
    manager->read_pointer(scan);
  }
}
