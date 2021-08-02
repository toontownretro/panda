/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapEntity.cxx
 * @author brian
 * @date 2021-07-08
 */

#include "mapEntity.h"
#include "bamReader.h"

IMPLEMENT_CLASS(MapEntity);

/**
 *
 */
MapEntity::
MapEntity() :
  _model_index(-1),
  _properties(nullptr)
{
}

/**
 *
 */
void MapEntity::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MapEntity::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_int16(_model_index);
  me.add_string(_class_name);

  if (_properties != nullptr) {
    me.add_bool(true);
    _properties->to_datagram(me);

  } else {
    me.add_bool(false);
  }
}

/**
 *
 */
void MapEntity::
fillin(DatagramIterator &scan, BamReader *manager) {
  _model_index = scan.get_int16();
  _class_name = scan.get_string();
  if (scan.get_bool()) {
    _properties = new PDXElement;
    _properties->from_datagram(scan);
  }
}

/**
 *
 */
TypedWritable *MapEntity::
make_from_bam(const FactoryParams &params) {
  MapEntity *ent = new MapEntity;
  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  ent->fillin(scan, manager);
  return ent;
}
