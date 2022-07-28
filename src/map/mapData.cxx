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
void LightDebugData::
write_datagram(Datagram &me) {
  me.add_uint32(_vertices.size());
  for (size_t i = 0; i < _vertices.size(); ++i) {
    _vertices[i].pos.write_datagram_fixed(me);
  }
  me.add_uint32(_triangles.size());
  for (size_t i = 0; i < _triangles.size(); ++i) {
    const Triangle &tri = _triangles[i];
    me.add_int32(tri.vert0);
    me.add_int32(tri.vert1);
    me.add_int32(tri.vert2);
  }
  me.add_uint32(_kd_nodes.size());
  for (size_t i = 0; i < _kd_nodes.size(); ++i) {
    const KDNode &node = _kd_nodes[i];
    me.add_int32(node.first_tri);
    me.add_int32(node.num_tris);
    me.add_int32(node.back_child);
    me.add_int32(node.front_child);
    node.mins.write_datagram_fixed(me);
    node.maxs.write_datagram_fixed(me);
    for (int j = 0; j < 6; ++j) {
      me.add_int32(node.neighbors[j]);
    }
    me.add_uint8(node.axis);
    me.add_float32(node.dist);
  }
  me.add_uint32(_tri_list.size());
  for (int itri : _tri_list) {
    me.add_int32(itri);
  }
}

/**
 *
 */
void LightDebugData::
read_datagram(DatagramIterator &scan) {
  _vertices.resize(scan.get_uint32());
  for (size_t i = 0; i < _vertices.size(); ++i) {
    _vertices[i].pos.read_datagram_fixed(scan);
  }
  _triangles.resize(scan.get_uint32());
  for (size_t i = 0; i < _triangles.size(); ++i) {
    Triangle &tri = _triangles[i];
    tri.vert0 = scan.get_int32();
    tri.vert1 = scan.get_int32();
    tri.vert2 = scan.get_int32();
  }
  _kd_nodes.resize(scan.get_uint32());
  for (size_t i = 0; i < _kd_nodes.size(); ++i) {
    KDNode &node = _kd_nodes[i];
    node.first_tri = scan.get_int32();
    node.num_tris = scan.get_int32();
    node.back_child = scan.get_int32();
    node.front_child = scan.get_int32();
    node.mins.read_datagram_fixed(scan);
    node.maxs.read_datagram_fixed(scan);
    for (int j = 0; j < 6; ++j) {
      node.neighbors[j] = scan.get_int32();
    }
    node.axis = scan.get_uint8();
    node.dist = scan.get_float32();
  }
  _tri_list.resize(scan.get_uint32());
  for (size_t i = 0; i < _tri_list.size(); ++i) {
    _tri_list[i] = scan.get_int32();
  }
}

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

  me.add_uint32(_models.size());
  for (size_t i = 0; i < _models.size(); ++i) {
    const MapModel *model = &_models[i];

    // Write pointer to GeomNode containing renderable geometry
    // for this model.
    manager->write_pointer(me, model->_geom_node);

    model->_mins.write_datagram_fixed(me);
    model->_maxs.write_datagram_fixed(me);

    // Write cooked physics triangle mesh.
    me.add_uint32(model->_tri_mesh_data.size());
    if (model->_tri_mesh_data.size() > 0u) {
      me.append_data(model->_tri_mesh_data.v());
    }

    // Surface props used by triangles of the physics mesh.
    me.add_uint16(model->_phys_surface_props.size());
    for (size_t j = 0; j < model->_phys_surface_props.size(); ++j) {
      me.add_string(model->_phys_surface_props[j]);
    }

    // Write the cooked convex mesh pieces for volume entities.
    me.add_uint16(model->_convex_mesh_data.size());
    for (size_t j = 0; j < model->_convex_mesh_data.size(); ++j) {
      const CPTA_uchar &cm_data = model->_convex_mesh_data[j];
      me.add_uint32(cm_data.size());
      if (cm_data.size() > 0u) {
        me.append_data(cm_data.v());
      }
    }
  }

  me.add_int32(_3d_sky_model);

  manager->write_pointer(me, _cluster_tree);

  me.add_uint32(_cluster_pvs.size());
  for (size_t i = 0; i < _cluster_pvs.size(); i++) {
    const AreaClusterPVS &pvs = _cluster_pvs[i];
    me.add_uint32(pvs._pvs.size());
    for (size_t j = 0; j < pvs._pvs.size(); j++) {
      me.add_int32(pvs._pvs[j]);
    }
    me.add_bool(pvs._3d_sky_cluster);
    me.add_uint32(pvs._box_bounds.size());
    for (size_t j = 0; j < pvs._box_bounds.size(); ++j) {
      pvs._box_bounds[j].write_datagram(me);
    }
  }

  me.add_uint16(_cube_maps.size());
  for (size_t i = 0; i < _cube_maps.size(); i++) {
    const MapCubeMap &cm = _cube_maps[i];
    manager->write_pointer(me, cm._texture);
    cm._pos.write_datagram(me);
    me.add_uint16(cm._size);
  }

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

  _light_debug_data.write_datagram(me);

  me.add_uint32(_static_props.size());
  for (const MapStaticProp &prop : _static_props) {
    me.add_string(prop._model_filename.get_fullpath());
    me.add_int8(prop._skin);
    prop._pos.write_datagram(me);
    prop._hpr.write_datagram(me);
    me.add_bool(prop._solid);

    me.add_uint32(prop._geom_vertex_lighting.size());
    for (const CPT(GeomVertexArrayData) &array : prop._geom_vertex_lighting) {
      manager->write_pointer(me, array);
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

  size_t num_models = scan.get_uint32();
  _models.resize(num_models);
  for (size_t i = 0; i < num_models; ++i) {
    MapModel *model = &_models[i];

    // GeomNode pointer.
    manager->read_pointer(scan);

    model->_mins.read_datagram_fixed(scan);
    model->_maxs.read_datagram_fixed(scan);

    // Physics triangle mesh data.
    PTA_uchar tri_mesh_data;
    tri_mesh_data.resize(scan.get_uint32());
    scan.extract_bytes(tri_mesh_data.p(), tri_mesh_data.size());
    model->_tri_mesh_data = tri_mesh_data;

    // Surface props.
    model->_phys_surface_props.resize(scan.get_uint16());
    for (size_t j = 0; j < model->_phys_surface_props.size(); ++j) {
      model->_phys_surface_props[j] = scan.get_string();
    }

    // Convex mesh pieces.
    model->_convex_mesh_data.resize(scan.get_uint16());
    for (size_t j = 0; j < model->_convex_mesh_data.size(); ++j) {
      PTA_uchar cm_data;
      cm_data.resize(scan.get_uint32());
      scan.extract_bytes(cm_data.p(), cm_data.size());
      model->_convex_mesh_data[j] = cm_data;
    }
  }

  _3d_sky_model = scan.get_int32();

  manager->read_pointer(scan); // _cluster_tree

  _cluster_pvs.resize(scan.get_uint32());
  for (size_t i = 0; i < _cluster_pvs.size(); i++) {
    AreaClusterPVS &pvs = _cluster_pvs[i];
    pvs._pvs.resize(scan.get_uint32());
    for (size_t j = 0; j < pvs._pvs.size(); j++) {
      pvs._pvs[j] = scan.get_int32();
    }
    pvs._3d_sky_cluster = scan.get_bool();
    pvs._box_bounds.resize(scan.get_uint32());
    for (size_t j = 0; j < pvs._box_bounds.size(); ++j) {
      pvs._box_bounds[j].read_datagram(scan);
    }
  }

  _cube_maps.resize(scan.get_uint16());
  for (size_t i = 0; i < _cube_maps.size(); i++) {
    MapCubeMap &cm = _cube_maps[i];
    manager->read_pointer(scan);
    cm._pos.read_datagram(scan);
    cm._size = scan.get_uint16();
  }

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

  _light_debug_data.read_datagram(scan);

  _static_props.resize(scan.get_uint32());
  for (MapStaticProp &prop : _static_props) {
    prop._model_filename = scan.get_string();
    prop._skin = scan.get_int8();
    prop._pos.read_datagram(scan);
    prop._hpr.read_datagram(scan);
    prop._solid = scan.get_bool();

    prop._geom_vertex_lighting.resize(scan.get_uint32());
    manager->read_pointers(scan, prop._geom_vertex_lighting.size());
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

  for (size_t i = 0; i < _models.size(); ++i) {
    _models[i]._geom_node = DCAST(GeomNode, p_list[pi++]);
  }

  _cluster_tree = DCAST(SpatialPartition, p_list[pi++]);

  for (size_t i = 0; i < _cube_maps.size(); i++) {
    _cube_maps[i]._texture = DCAST(Texture, p_list[pi++]);
  }

  for (size_t i = 0; i < _lights.size(); i++) {
    pi += _lights[i].complete_pointers(p_list + pi, manager);
  }

  for (MapStaticProp &prop : _static_props) {
    for (CPT(GeomVertexArrayData) &array : prop._geom_vertex_lighting) {
      array = DCAST(GeomVertexArrayData, p_list[pi++]);
    }
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

  const MapModel *model = &_models[0];

  for (size_t j = 0; j < model->_geom_node->get_num_geoms(); ++j) {
    const Geom *geom = model->_geom_node->get_geom(j);
    const RenderState *state = model->_geom_node->get_geom_state(j);
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
