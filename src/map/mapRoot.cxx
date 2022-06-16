/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRoot.cxx
 * @author brian
 * @date 2021-07-09
 */

#include "mapRoot.h"

IMPLEMENT_CLASS(MapRoot);

/**
 *
 */
MapRoot::
MapRoot(MapData *data) :
  PandaNode("map-root"),
  _data(data)
{
}

/**
 *
 */
void MapRoot::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MapRoot::
write_datagram(BamWriter *manager, Datagram &me) {
  PandaNode::write_datagram(manager, me);

  manager->write_pointer(me, _data);
}

/**
 *
 */
void MapRoot::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);
}

/**
 *
 */
int MapRoot::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);
  _data = DCAST(MapData, p_list[pi++]);
  return pi;
}

/**
 *
 */
TypedWritable *MapRoot::
make_from_bam(const FactoryParams &params) {
  MapRoot *node = new MapRoot;
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  node->fillin(scan, manager);
  return node;
}

/**
 *
 */
PandaNode *MapRoot::
make_copy() const {
  return new MapRoot(*this);
}

/**
 *
 */
MapRoot::
MapRoot() :
  PandaNode("map-root"),
  _data(nullptr)
{
}

/**
 *
 */
MapRoot::
MapRoot(const MapRoot &copy) :
  PandaNode(copy),
  _data(copy._data)
{
}
