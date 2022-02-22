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
#include "renderState.h"
#include "geomNode.h"
#include "geom.h"
#include "materialAttrib.h"
#include "material.h"
#include "light.h"
#include "directionalLight.h"

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
    WRITE_PTA(manager, me, IPD_uchar::write_datagram, _model_phys_data[i]._phys_mesh_data);
    me.add_uint16(_model_phys_data[i]._phys_surface_props.size());
    for (size_t j = 0; j < _model_phys_data[i]._phys_surface_props.size(); ++j) {
      me.add_string(_model_phys_data[i]._phys_surface_props[j]);
    }
  }

  manager->write_pointer(me, _cluster_tree);

  me.add_uint32(_cluster_pvs.size());
  for (size_t i = 0; i < _cluster_pvs.size(); i++) {
    const AreaClusterPVS &pvs = _cluster_pvs[i];
    me.add_uint32(pvs._pvs.size());
    for (size_t j = 0; j < pvs._pvs.size(); j++) {
      me.add_int32(pvs._pvs[j]);
    }
    pvs._mesh_groups.write_datagram(manager, me);
    me.add_bool(pvs._3d_sky_cluster);
    me.add_uint32(pvs._box_bounds.size());
    for (size_t j = 0; j < pvs._box_bounds.size(); ++j) {
      pvs._box_bounds[j].write_datagram(me);
    }
  }

  me.add_uint32(_mesh_groups.size());
  for (size_t i = 0; i < _mesh_groups.size(); i++) {
    const MapMeshGroup &group = _mesh_groups[i];
    group._clusters.write_datagram(manager, me);
    me.add_bool(group._in_3d_skybox);
    manager->write_pointer(me, group._geom_node);
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

  WRITE_PTA(manager, me, IPD_uchar::write_datagram, _steam_audio_scene_data.verts);
  WRITE_PTA(manager, me, IPD_uchar::write_datagram, _steam_audio_scene_data.tris);
  WRITE_PTA(manager, me, IPD_uchar::write_datagram, _steam_audio_scene_data.tri_materials);
  WRITE_PTA(manager, me, IPD_uchar::write_datagram, _steam_audio_scene_data.materials);
  WRITE_PTA(manager, me, IPD_uchar::write_datagram, _steam_audio_probe_data);
  WRITE_PTA(manager, me, IPD_uchar::write_datagram, _steam_audio_pathing_probe_data);
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
    _model_phys_data[i]._phys_mesh_data = data;
    _model_phys_data[i]._phys_surface_props.resize(scan.get_uint16());
    for (size_t j = 0; j < _model_phys_data[i]._phys_surface_props.size(); ++j) {
      _model_phys_data[i]._phys_surface_props[j] = scan.get_string();
    }
  }

  manager->read_pointer(scan); // _cluster_tree

  _cluster_pvs.resize(scan.get_uint32());
  for (size_t i = 0; i < _cluster_pvs.size(); i++) {
    AreaClusterPVS &pvs = _cluster_pvs[i];
    pvs._pvs.resize(scan.get_uint32());
    for (size_t j = 0; j < pvs._pvs.size(); j++) {
      pvs._pvs[j] = scan.get_int32();
    }
    pvs._mesh_groups.read_datagram(scan, manager);
    pvs._3d_sky_cluster = scan.get_bool();
    pvs._box_bounds.resize(scan.get_uint32());
    for (size_t j = 0; j < pvs._box_bounds.size(); ++j) {
      pvs._box_bounds[j].read_datagram(scan);
    }
  }

  _mesh_groups.resize(scan.get_uint32());
  for (size_t i = 0; i < _mesh_groups.size(); i++) {
    MapMeshGroup &group = _mesh_groups[i];
    group._clusters.read_datagram(scan, manager);
    group._in_3d_skybox = scan.get_bool();
    manager->read_pointer(scan); // _geom_node
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

  PTA_uchar scene_verts, scene_tris, scene_tri_materials, scene_materials, probe_data, pathing_data;
  READ_PTA(manager, scan, IPD_uchar::read_datagram, scene_verts);
  READ_PTA(manager, scan, IPD_uchar::read_datagram, scene_tris);
  READ_PTA(manager, scan, IPD_uchar::read_datagram, scene_tri_materials);
  READ_PTA(manager, scan, IPD_uchar::read_datagram, scene_materials);
  READ_PTA(manager, scan, IPD_uchar::read_datagram, probe_data);
  READ_PTA(manager, scan, IPD_uchar::read_datagram, pathing_data);
  _steam_audio_scene_data.verts = scene_verts;
  _steam_audio_scene_data.tris = scene_tris;
  _steam_audio_scene_data.tri_materials = scene_tri_materials;
  _steam_audio_scene_data.materials = scene_materials;
  _steam_audio_probe_data = probe_data;
  _steam_audio_pathing_probe_data = pathing_data;
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

  _cluster_tree = DCAST(SpatialPartition, p_list[pi++]);

  for (size_t i = 0; i < _mesh_groups.size(); i++) {
    _mesh_groups[i]._geom_node = DCAST(GeomNode, p_list[pi++]);
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

/**
 *
 */
RayTraceScene *MapData::
get_trace_scene() const {
  if (_trace_scene == nullptr) {
    ((MapData *)this)->build_trace_scene();
  }
  return _trace_scene;
}

/**
 *
 */
void MapData::
build_trace_scene() {
  RayTrace::initialize();

  _trace_scene = new RayTraceScene;
  _trace_scene->set_build_quality(RayTraceScene::BUILD_QUALITY_HIGH);

  for (size_t i = 0; i < _mesh_groups.size(); ++i) {
    if (_mesh_groups[i]._geom_node == nullptr) {
      continue;
    }
    for (size_t j = 0; j < _mesh_groups[i]._geom_node->get_num_geoms(); ++j) {
      const Geom *geom = _mesh_groups[i]._geom_node->get_geom(j);
      const RenderState *state = _mesh_groups[i]._geom_node->get_geom_state(j);
      const MaterialAttrib *mattr;
      state->get_attrib_def(mattr);
      Material *mat = mattr->get_material();
      unsigned int mask = 1;
      if (mat != nullptr) {
        if ((mat->_attrib_flags & Material::F_transparency) != 0 && mat->_transparency_mode != 0) {
          continue;
        }
        if (mat->has_tag("compile_sky")) {
          mask = 2;
        }
      }
      PT(RayTraceTriangleMesh) mesh = new RayTraceTriangleMesh;
      mesh->add_triangles_from_geom(geom);
      mesh->set_mask(mask);
      mesh->build();
      _trace_scene->add_geometry(mesh);
      _trace_meshes.push_back(mesh);
    }
  }

  _trace_scene->update();

  for (size_t i = 0; i < _lights.size(); ++i) {
    if (_lights[i].node()->is_of_type(DirectionalLight::get_class_type())) {
      _dir_light = _lights[i];
      LQuaternion q;
      q.set_hpr(_dir_light.get_hpr());
      _dir_light_dir = q.get_forward();
      break;
    }
  }
}

/**
 *
 */
void MapData::
check_lighting_pvs() {
  if (_built_light_pvs) {
    return;
  }

  _light_pvs.resize(_cluster_pvs.size());
  _probe_pvs.resize(_cluster_pvs.size());
  _cube_map_pvs.resize(_cluster_pvs.size());

  for (size_t i = 0; i < _lights.size(); ++i) {
    if (_lights[i].node()->is_of_type(DirectionalLight::get_class_type())) {
      continue;
    }

    int cluster = _cluster_tree->get_leaf_value_from_point(_lights[i].get_net_transform()->get_pos());
    if (cluster < 0) {
      continue;
    }
    const AreaClusterPVS *pvs = &_cluster_pvs[cluster];
    for (size_t j = 0; j < pvs->get_num_visible_clusters(); ++j) {
      _light_pvs[pvs->get_visible_cluster(j)].push_back((int)i);
    }
  }

  for (size_t i = 0; i < _ambient_probes.size(); ++i) {
    const MapAmbientProbe *probe = &_ambient_probes[i];

    int cluster = _cluster_tree->get_leaf_value_from_point(probe->_pos);
    if (cluster < 0) {
      continue;
    }
    _probe_pvs[cluster].push_back((int)i);
    //const AreaClusterPVS *pvs = &_cluster_pvs[cluster];
    //for (size_t j = 0; j < pvs->get_num_visible_clusters(); ++j) {
    //  _probe_pvs[pvs->get_visible_cluster(j)].push_back((int)i);
    //}
  }

  for (size_t i = 0; i < _cube_maps.size(); ++i) {
    const MapCubeMap *cm = &_cube_maps[i];

    int cluster = _cluster_tree->get_leaf_value_from_point(cm->_pos);
    if (cluster < 0) {
      continue;
    }
    const AreaClusterPVS *pvs = &_cluster_pvs[cluster];
    for (size_t j = 0; j < pvs->get_num_visible_clusters(); ++j) {
      _cube_map_pvs[pvs->get_visible_cluster(j)].push_back((int)i);
    }
  }

  _built_light_pvs = true;
}
