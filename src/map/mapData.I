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
 * Marks the indicated mesh group index as residing in this cluster.
 */
INLINE void AreaClusterPVS::
set_mesh_group(int index) {
  _mesh_groups.set_bit(index);
}

/**
 * Clears the indicated mesh group from this cluster.
 */
INLINE void AreaClusterPVS::
clear_mesh_group(int index) {
  _mesh_groups.clear_bit(index);
}

/**
 * Returns the BitArray of mesh groups that reside in this cluster.
 */
INLINE const BitArray &AreaClusterPVS::
get_mesh_groups() const {
  return _mesh_groups;
}

/**
 *
 */
INLINE void MapMeshGroup::
set_cluster(int cluster) {
  _clusters.set_bit(cluster);
}

/**
 *
 */
INLINE void MapMeshGroup::
clear_cluster(int cluster) {
  _clusters.clear_bit(cluster);
}

/**
 *
 */
INLINE const BitArray &MapMeshGroup::
get_clusters() const {
  return _clusters;
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
MapEntity *MapData::
get_entity(int n) const {
  nassertr(n >= 0 && n < (int)_entities.size(), nullptr);
  return _entities[n];
}

/**
 *
 */
INLINE void MapData::
add_model_phys_data(CPTA_uchar data) {
  _model_phys_data.push_back(data);
}

/**
 *
 */
INLINE int MapData::
get_num_model_phys_datas() const {
  return (int)_model_phys_data.size();
}

/**
 *
 */
INLINE CPTA_uchar MapData::
get_model_phys_data(int n) const {
  nassertr(n >= 0 && n < (int)_model_phys_data.size(), PTA_uchar::empty_array(0));
  return _model_phys_data[n];
}

/**
 * Sets the area cluster tree associated with this map.
 */
INLINE void MapData::
set_area_cluster_tree(KDTree &&tree) {
  _cluster_tree = std::move(tree);
}

/**
 * Returns the area cluster tree associated with this map.
 */
INLINE const KDTree *MapData::
get_area_cluster_tree() const {
  return &_cluster_tree;
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
add_mesh_group(const MapMeshGroup &group) {
  _mesh_groups.push_back(group);
}

/**
 *
 */
INLINE int MapData::
get_num_mesh_groups() const {
  return (int)_mesh_groups.size();
}

/**
 *
 */
INLINE const MapMeshGroup *MapData::
get_mesh_group(int n) const {
  nassertr(n >= 0 && n < (int)_mesh_groups.size(), nullptr);
  return &_mesh_groups[n];
}
