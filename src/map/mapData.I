/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapData.I
 * @author brian
 * @date 2021-07-08
 */

/**
 *
 */
INLINE void AreaClusterPVS::
add_visible_cluster(int cluster) {
  _pvs.push_back(cluster);
}

/**
 *
 */
INLINE size_t AreaClusterPVS::
get_num_visible_clusters() const {
  return _pvs.size();
}

/**
 *
 */
INLINE int AreaClusterPVS::
get_visible_cluster(size_t n) const {
  nassertr(n < _pvs.size(), 0);
  return _pvs[n];
}

/**
 * Returns the number of area boxes that make up the cluster.
 */
INLINE size_t AreaClusterPVS::
get_num_boxes() const {
  // The vector stores mins and maxs of each box contiguously in one array.
  return _box_bounds.size() / 2;
}

/**
 * Fills in the minimum and maximum coordinate of the indicated area box.
 */
INLINE void AreaClusterPVS::
get_box_bounds(size_t n, LPoint3 &mins, LPoint3 &maxs) const {
  nassertv(n < _box_bounds.size() / 2);
  mins = _box_bounds[n * 2];
  maxs = _box_bounds[n * 2 + 1];
}

/**
 *
 */
INLINE void MapData::
add_entity(MapEntity *ent) {
  _entities.push_back(ent);
}

/**
 *
 */
INLINE int MapData::
get_num_entities() const {
  return (int)_entities.size();
}

/**
 *
 */
INLINE MapEntity *MapData::
get_entity(int n) const {
  nassertr(n >= 0 && n < (int)_entities.size(), nullptr);
  return _entities[n];
}

/**
 *
 */
INLINE void MapData::
add_model(const MapModel &model) {
  _models.push_back(model);
}

/**
 *
 */
INLINE int MapData::
get_num_models() const {
  return (int)_models.size();
}

/**
 *
 */
INLINE const MapModel *MapData::
get_model(int n) const {
  nassertr(n >= 0 && n < (int)_models.size(), nullptr);
  return &_models[n];
}

/**
 * Returns the index of the MapModel that contains the 3-D skybox
 * geometry.
 */
INLINE int MapData::
get_3d_sky_model_index() const {
  return _3d_sky_model;
}

/**
 * Sets the area cluster tree associated with this map.
 */
INLINE void MapData::
set_area_cluster_tree(SpatialPartition *tree) {
  _cluster_tree = tree;
}

/**
 * Returns the area cluster tree associated with this map.
 */
INLINE const SpatialPartition *MapData::
get_area_cluster_tree() const {
  return _cluster_tree;
}

/**
 *
 */
INLINE void MapData::
add_cluster_pvs(const AreaClusterPVS &pvs) {
  _cluster_pvs.push_back(pvs);
}

/**
 *
 */
INLINE int MapData::
get_num_clusters() const {
  return (int)_cluster_pvs.size();
}

/**
 *
 */
INLINE const AreaClusterPVS *MapData::
get_cluster_pvs(int cluster) const {
  nassertr(cluster >= 0 && cluster < (int)_cluster_pvs.size(), nullptr);
  return &_cluster_pvs[cluster];
}

/**
 *
 */
INLINE void MapData::
add_cube_map(Texture *tex, const LPoint3 &pos, int size) {
  MapCubeMap cm;
  cm._texture = tex;
  cm._pos = pos;
  cm._size = size;
  _cube_maps.push_back(cm);
}

/**
 *
 */
INLINE int MapData::
get_num_cube_maps() const {
  return (int)_cube_maps.size();
}

/**
 *
 */
INLINE const MapCubeMap *MapData::
get_cube_map(int n) const {
  nassertr(n >= 0 && n < (int)_cube_maps.size(), nullptr);
  return &_cube_maps[n];
}

/**
 *
 */
INLINE MapCubeMap *MapData::
modify_cube_map(int n) {
  nassertr(n >= 0 && n < (int)_cube_maps.size(), nullptr);
  return &_cube_maps[n];
}

/**
 *
 */
//INLINE void MapData::
//set_cube_map_tree(KDTree &&tree) {
//  _cube_map_tree = std::move(tree);
//}

/**
 *
 */
//INLINE const KDTree *MapData::
//get_cube_map_tree() const {
//  return &_cube_map_tree;
//}

/**
 *
 */
INLINE void MapData::
add_light(NodePath light) {
  _lights.push_back(light);
}

/**
 *
 */
INLINE int MapData::
get_num_lights() const {
  return (int)_lights.size();
}

/**
 *
 */
INLINE NodePath MapData::
get_light(int n) const {
  nassertr(n >= 0 && n < (int)_lights.size(), NodePath());
  return _lights[n];
}

/**
 *
 */
INLINE void MapData::
add_ambient_probe(const MapAmbientProbe &probe) {
  _ambient_probes.push_back(probe);
}

/**
 *
 */
INLINE int MapData::
get_num_ambient_probes() const {
  return (int)_ambient_probes.size();
}

/**
 *
 */
INLINE const MapAmbientProbe *MapData::
get_ambient_probe(int n) const {
  nassertr(n >= 0 && n < (int)_ambient_probes.size(), nullptr);
  return &_ambient_probes[n];
}

/**
 *
 */
INLINE const SteamAudioSceneData &MapData::
get_steam_audio_scene_data() const {
  return _steam_audio_scene_data;
}

/**
 *
 */
INLINE CPTA_uchar MapData::
get_steam_audio_probe_data() const {
  return _steam_audio_probe_data;
}

/**
 *
 */
INLINE CPTA_uchar MapData::
get_steam_audio_pathing_probe_data() const {
  return _steam_audio_pathing_probe_data;
}

/**
 *
 */
INLINE const LightDebugData *MapData::
get_light_debug_data() const {
  return &_light_debug_data;
}

/**
 *
 */
INLINE void MapData::
set_cam(NodePath cam) {
  _cam = cam;
}
