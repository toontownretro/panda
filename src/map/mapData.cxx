/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapData.cxx
 * @author brian
 * @date 2021-07-08
 */

#include "mapData.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramChar.h"

IMPLEMENT_CLASS(MapData);

/**
 *
 */
void MapData::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
void MapData::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_uint16(_entities.size());
  for (size_t i = 0; i < _entities.size(); i++) {
    manager->write_pointer(me, _entities[i]);
  }

  me.add_uint16(_model_phys_data.size());
  for (size_t i = 0; i < _model_phys_data.size(); i++) {
    WRITE_PTA(manager, me, IPD_uchar::write_datagram, _model_phys_data[i]);
  }

  _cluster_tree.write_datagram(me);

  me.add_uint32(_cluster_pvs.size());
  for (size_t i = 0; i < _cluster_pvs.size(); i++) {
    const AreaClusterPVS &pvs = _cluster_pvs[i];
    me.add_uint32(pvs._pvs.size());
    for (size_t j = 0; j < pvs._pvs.size(); j++) {
      me.add_int32(pvs._pvs[j]);
    }
    pvs._mesh_groups.write_datagram(manager, me);
  }

  me.add_uint32(_mesh_groups.size());
  for (size_t i = 0; i < _mesh_groups.size(); i++) {
    const MapMeshGroup &group = _mesh_groups[i];
    group._clusters.write_datagram(manager, me);
  }

  me.add_uint16(_cube_maps.size());
  for (size_t i = 0; i < _cube_maps.size(); i++) {
    const MapCubeMap &cm = _cube_maps[i];
    manager->write_pointer(me, cm._texture);
    cm._pos.write_datagram(me);
  }

  //_cube_map_tree.write_datagram(me);

  me.add_uint16(_lights.size());
  for (size_t i = 0; i < _lights.size(); i++) {
    _lights[i].write_datagram(manager, me);
  }

  me.add_uint32(_ambient_probes.size());
  for (size_t i = 0; i < _ambient_probes.size(); i++) {
    const MapAmbientProbe &probe = _ambient_probes[i];
    probe._pos.write_datagram(me);
    for (int j = 0; j < 9; j++) {
      probe._color[j].write_datagram(me);
    }
  }
}

/**
 *
 */
void MapData::
fillin(DatagramIterator &scan, BamReader *manager) {
  size_t num_entities = scan.get_uint16();
  _entities.resize(num_entities);
  manager->read_pointers(scan, num_entities);

  size_t num_phys_datas = scan.get_uint16();
  _model_phys_data.resize(num_phys_datas);
  for (size_t i = 0; i < num_phys_datas; i++) {
    PTA_uchar data;
    READ_PTA(manager, scan, IPD_uchar::read_datagram, data);
    _model_phys_data[i] = data;
  }

  _cluster_tree.read_datagram(scan);

  _cluster_pvs.resize(scan.get_uint32());
  for (size_t i = 0; i < _cluster_pvs.size(); i++) {
    AreaClusterPVS &pvs = _cluster_pvs[i];
    pvs._pvs.resize(scan.get_uint32());
    for (size_t j = 0; j < pvs._pvs.size(); j++) {
      pvs._pvs[j] = scan.get_int32();
    }
    pvs._mesh_groups.read_datagram(scan, manager);
  }

  _mesh_groups.resize(scan.get_uint32());
  for (size_t i = 0; i < _mesh_groups.size(); i++) {
    MapMeshGroup &group = _mesh_groups[i];
    group._clusters.read_datagram(scan, manager);
  }

  _cube_maps.resize(scan.get_uint16());
  for (size_t i = 0; i < _cube_maps.size(); i++) {
    MapCubeMap &cm = _cube_maps[i];
    manager->read_pointer(scan);
    cm._pos.read_datagram(scan);
  }

  //_cube_map_tree.read_datagram(scan);

  _lights.resize(scan.get_uint16());
  for (size_t i = 0; i < _lights.size(); i++) {
    _lights[i].fillin(scan, manager);
  }

  _ambient_probes.resize(scan.get_uint32());
  for (size_t i = 0; i < _ambient_probes.size(); i++) {
    MapAmbientProbe &probe = _ambient_probes[i];
    probe._pos.read_datagram(scan);
    for (int j = 0; j < 9; j++) {
      probe._color[j].read_datagram(scan);
    }
  }
}

/**
 *
 */
int MapData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  for (size_t i = 0; i < _entities.size(); i++) {
    _entities[i] = DCAST(MapEntity, p_list[pi++]);
  }

  for (size_t i = 0; i < _cube_maps.size(); i++) {
    _cube_maps[i]._texture = DCAST(Texture, p_list[pi++]);
  }

  for (size_t i = 0; i < _lights.size(); i++) {
    pi += _lights[i].complete_pointers(p_list + pi, manager);
  }

  return pi;
}

/**
 *
 */
TypedWritable *MapData::
make_from_bam(const FactoryParams &params) {
  MapData *data = new MapData;
  BamReader *manager;
  DatagramIterator scan;
  parse_params(params, scan, manager);

  data->fillin(scan, manager);
  return data;
}
