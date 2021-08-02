/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapData.h
 * @author brian
 * @date 2021-07-08
 */

#ifndef MAPDATA_H
#define MAPDATA_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "pvector.h"
#include "mapEntity.h"
#include "factoryParams.h"
#include "kdTree.h"
#include "bitArray.h"

/**
 * PVS for a single area cluster.
 */
class EXPCL_PANDA_MAP AreaClusterPVS {
PUBLISHED:
  INLINE void add_visible_cluster(int cluster);
  INLINE size_t get_num_visible_clusters() const;
  INLINE int get_visible_cluster(size_t n) const;

  INLINE void set_mesh_group(int index);
  INLINE void clear_mesh_group(int index);
  INLINE const BitArray &get_mesh_groups() const;

public:
  vector_int _pvs;

  // Indices of mesh groups that reside in this cluster.
  // A 1 bit means the mesh group is in there.
  BitArray _mesh_groups;
};

class EXPCL_PANDA_MAP MapMeshGroup {
PUBLISHED:
  INLINE void set_cluster(int cluster);
  INLINE void clear_cluster(int cluster);
  INLINE const BitArray &get_clusters() const;

public:
  BitArray _clusters;
};

/**
 * The main data store for a map.
 */
class EXPCL_PANDA_MAP MapData : public TypedWritableReferenceCount {
  DECLARE_CLASS(MapData, TypedWritableReferenceCount);

PUBLISHED:
  MapData() = default;

  INLINE void add_entity(MapEntity *ent);
  INLINE int get_num_entities() const;
  INLINE MapEntity *get_entity(int n) const;

  INLINE void add_model_phys_data(CPTA_uchar data);
  INLINE int get_num_model_phys_datas() const;
  INLINE CPTA_uchar get_model_phys_data(int n) const;

  INLINE void set_area_cluster_tree(KDTree &&tree);
  INLINE const KDTree *get_area_cluster_tree() const;

  INLINE void add_cluster_pvs(const AreaClusterPVS &pvs);
  INLINE int get_num_clusters() const;
  INLINE const AreaClusterPVS *get_cluster_pvs(int cluster) const;

  INLINE void add_mesh_group(const MapMeshGroup &group);
  INLINE int get_num_mesh_groups() const;
  INLINE const MapMeshGroup *get_mesh_group(int n) const;

public:
  static void register_with_read_factory();

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

private:
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  pvector<PT(MapEntity)> _entities;
  pvector<CPTA_uchar> _model_phys_data;

  KDTree _cluster_tree;
  pvector<AreaClusterPVS> _cluster_pvs;

  pvector<MapMeshGroup> _mesh_groups;
};

#include "mapData.I"

#endif // MAPDATA_H
