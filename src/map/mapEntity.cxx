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

  nassertv(_connections.size() <= 255u);
  me.add_uint8(_connections.size());
  for (const Connection &conn : _connections) {
    me.add_string(conn._output_name);
    me.add_string(conn._target_name);
    me.add_string(conn._input_name);
    me.add_uint8(conn._parameters.size());
    for (const std::string &param : conn._parameters) {
      me.add_string(param);
    }
    me.add_float32(conn._delay);
    me.add_bool(conn._repeat);
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

  _connections.resize(scan.get_uint8());
  for (size_t i = 0; i < _connections.size(); ++i) {
    Connection &conn = _connections[i];
    conn._output_name = scan.get_string();
    conn._target_name = scan.get_string();
    conn._input_name = scan.get_string();
    conn._parameters.resize(scan.get_uint8());
    for (size_t j = 0; j < conn._parameters.size(); ++j) {
      conn._parameters[j] = scan.get_string();
    }
    conn._delay = scan.get_float32();
    conn._repeat = scan.get_bool();
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
