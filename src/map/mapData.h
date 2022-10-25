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
#include "spatialPartition.h"
#include "bitArray.h"
#include "texture.h"
#include "luse.h"
#include "nodePath.h"
#include "geomNode.h"
#include "rayTrace.h"
#include "rayTraceTriangleMesh.h"
#include "rayTraceScene.h"
#include "geomVertexArrayData.h"

class SteamAudioSceneData {
PUBLISHED:
  CPTA_uchar verts;
  CPTA_uchar tris;
  CPTA_uchar tri_materials;
  CPTA_uchar materials;
};

/**
 * PVS for a single area cluster.
 */
class EXPCL_PANDA_MAP AreaClusterPVS {
PUBLISHED:
  INLINE void add_visible_cluster(int cluster);
  INLINE size_t get_num_visible_clusters() const;
  INLINE int get_visible_cluster(size_t n) const;

  INLINE size_t get_num_boxes() const;
  INLINE void get_box_bounds(size_t n, LPoint3 &mins, LPoint3 &maxs) const;

  INLINE bool is_3d_sky_cluster() const { return _3d_sky_cluster; }

public:
  vector_int _pvs;

  // Cluster bounds for visualization purposes.
  pvector<LPoint3> _box_bounds;

  bool _3d_sky_cluster;
};

/**
 * A specular reflection probe placed at a point in the scene.
 * Texture is baked at map build time.
 */
class EXPCL_PANDA_MAP MapCubeMap {
PUBLISHED:
  INLINE void set_texture(Texture *tex) { _texture = tex; }
  INLINE Texture *get_texture() const { return _texture; }
  INLINE const LPoint3 &get_pos() const { return _pos; }
  INLINE int get_size() const { return _size; }

  PT(Texture) _texture;
  LPoint3 _pos;
  int _size;
};

/**
 * An ambient light probe in the scene.  Encoded using spherical harmonics.
 * Applied to dynamic models.
 */
class EXPCL_PANDA_MAP MapAmbientProbe {
PUBLISHED:
  LPoint3 _pos;

  INLINE const LVecBase3 &get_color(int i) const { return _color[i]; }
  INLINE const LPoint3 &get_pos() const { return _pos; }

public:
  LVecBase3 _color[9];
};

/**
 *
 */
class EXPCL_PANDA_MAP MapModel {
PUBLISHED:
  MapModel() = default;
  ~MapModel() = default;

  class CollisionGroup {
  public:
    std::string _collide_type;
    CPTA_uchar _tri_mesh_data;
    // The mesh data indexes into this list.
    // When the map is loaded, we create a PhysMaterial corresponding to
    // the surfaceprop name in show code and add them to the PhysShape
    // created for this model phys data in the same order.
    vector_string _phys_surface_props;

  PUBLISHED:
    INLINE const std::string &get_collide_type() const {
      return _collide_type;
    }
    INLINE CPTA_uchar get_tri_mesh_data() const {
      return _tri_mesh_data;
    }
    INLINE size_t get_num_surface_props() const {
      return _phys_surface_props.size();
    }
    INLINE std::string get_surface_prop(size_t n) const {
      return _phys_surface_props[n];
    }
  };

  INLINE GeomNode *get_geom_node() const { return _geom_node; }

  INLINE const LPoint3 &get_mins() const { return _mins; }
  INLINE const LPoint3 &get_maxs() const { return _maxs; }

  INLINE int get_num_tri_groups() const {
    return (int)_tri_groups.size();
  }
  INLINE const CollisionGroup *get_tri_group(int n) const {
    return &_tri_groups[n];
  }

  INLINE int get_num_convex_meshes() const { return (int)_convex_mesh_data.size(); }
  INLINE CPTA_uchar get_convex_mesh_data(int n) const {
    nassertr(n >= 0 && n < (int)_convex_mesh_data.size(), CPTA_uchar());
    return _convex_mesh_data[n];
  }

public:
  PT(GeomNode) _geom_node;

  LPoint3 _mins, _maxs;

  // Collision triangle meshes, grouped by collide type.
  // Allows show code to assign specific collide masks per
  // collision group type.
  pvector<CollisionGroup> _tri_groups;
  pvector<CPTA_uchar> _convex_mesh_data;
};

/**
 *
 */
class EXPCL_PANDA_MAP LightDebugData {
PUBLISHED:
  LightDebugData() = default;

  class Vertex {
  PUBLISHED:
    LPoint3 pos;
  };
  class Triangle {
  PUBLISHED:
    int vert0;
    int vert1;
    int vert2;
  };
  class KDNode {
  PUBLISHED:
    int first_tri;
    int num_tris;
    int back_child;
    int front_child;
    LPoint3 mins;
    LPoint3 maxs;
    int neighbors[6];
    unsigned char axis;
    float dist;

    INLINE int get_neighbor(int n) const {
      return neighbors[n];
    }
  };

  INLINE const Vertex *get_vert(int n) const {
    return &_vertices[n];
  }
  INLINE int get_num_vertices() const {
    return (int)_vertices.size();
  }

  INLINE int get_num_tris() const {
    return (int)_triangles.size();
  }
  INLINE const Triangle *get_tri(int n) const {
    return &_triangles[n];
  }

  INLINE int get_num_kd_nodes() const {
    return (int)_kd_nodes.size();
  }
  INLINE const KDNode *get_kd_node(int n) const {
    return &_kd_nodes[n];
  }

  INLINE int get_kd_tri(int n) const {
    return _tri_list[n];
  }

  void write_datagram(Datagram &me);
  void read_datagram(DatagramIterator &scan);

public:
  typedef pvector<Vertex> Vertices;
  typedef pvector<Triangle> Triangles;
  typedef pvector<KDNode> KDNodes;

  Vertices _vertices;
  Triangles _triangles;
  KDNodes _kd_nodes;
  vector_int _tri_list;
};

class EXPCL_PANDA_MAP MapStaticProp {
PUBLISHED:
  ~MapStaticProp() = default;

  enum Flags {
    F_none = 0,
    F_no_shadows = 1,
    F_no_vertex_lighting = 2,
  };

  INLINE const Filename &get_model_filename() const { return _model_filename; }
  INLINE int get_skin() const { return _skin; }
  INLINE const LPoint3 &get_pos() const { return _pos; }
  INLINE const LVecBase3 &get_hpr() const { return _hpr; }
  INLINE bool get_solid() const { return _solid; }
  INLINE const GeomVertexArrayData *get_vertex_lighting(int n) const {
    nassertr(n >= 0 && n < (int)_geom_vertex_lighting.size(), nullptr);
    return _geom_vertex_lighting[n];
  }

public:
  MapStaticProp() = default;

  Filename _model_filename;
  int _skin;
  LPoint3 _pos;
  LVecBase3 _hpr;
  bool _solid;

  unsigned int _flags;

  // One entry for each Geom of the prop.  Pointers will be
  // shared if the Geoms share a GeomVertexData.
  typedef pvector<CPT(GeomVertexArrayData)> GeomVertexLighting;
  GeomVertexLighting _geom_vertex_lighting;
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

  INLINE void add_model(const MapModel &model);
  INLINE int get_num_models() const;
  INLINE const MapModel *get_model(int n) const;

  INLINE int get_3d_sky_model_index() const;

  INLINE void set_area_cluster_tree(SpatialPartition *tree);
  INLINE const SpatialPartition *get_area_cluster_tree() const;

  INLINE void add_cluster_pvs(const AreaClusterPVS &pvs);
  INLINE int get_num_clusters() const;
  INLINE const AreaClusterPVS *get_cluster_pvs(int cluster) const;

  INLINE void add_cube_map(Texture *tex, const LPoint3 &pos, int size);
  INLINE int get_num_cube_maps() const;
  INLINE const MapCubeMap *get_cube_map(int n) const;
  INLINE MapCubeMap *modify_cube_map(int n);

  INLINE void add_light(NodePath light);
  INLINE int get_num_lights() const;
  INLINE NodePath get_light(int n) const;

  INLINE void add_ambient_probe(const MapAmbientProbe &probe);
  INLINE int get_num_ambient_probes() const;
  INLINE const MapAmbientProbe *get_ambient_probe(int n) const;

  INLINE const SteamAudioSceneData &get_steam_audio_scene_data() const;
  INLINE CPTA_uchar get_steam_audio_probe_data() const;
  INLINE CPTA_uchar get_steam_audio_pathing_probe_data() const;

  INLINE const LightDebugData *get_light_debug_data() const;

  INLINE void set_cam(NodePath cam);

  INLINE NodePath get_dir_light() const { return _dir_light; }

  INLINE size_t get_num_static_props() const { return _static_props.size(); }
  INLINE const MapStaticProp *get_static_prop(size_t n) const {
    nassertr(n < _static_props.size(), nullptr);
    return &_static_props[n];
  }

  RayTraceScene *get_trace_scene() const;

  void check_lighting_pvs();

  void build_trace_scene();

public:
  static void register_with_read_factory();

  virtual void write_datagram(BamWriter *manager, Datagram &me) override;
  virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager) override;

private:
  static TypedWritable *make_from_bam(const FactoryParams &params);

private:
  pvector<PT(MapEntity)> _entities;
  pvector<MapModel> _models;
  int _3d_sky_model = -1;

  PT(SpatialPartition) _cluster_tree;
  pvector<AreaClusterPVS> _cluster_pvs;

  pvector<MapCubeMap> _cube_maps;
  // For doing nearest neighbor cube map search.
  //KDTree _cube_map_tree;

  pvector<MapAmbientProbe> _ambient_probes;

  pvector<NodePath> _lights;
  NodePath _dir_light;
  LVector3 _dir_light_dir;

  NodePath _cam;

  // For each vis cluster, vector of indices into _lights for lights that are
  // in that cluster's PVS.  Same for ambient probes and cube maps.
  bool _built_light_pvs = false;
  pvector<vector_int> _light_pvs;
  pvector<vector_int> _probe_pvs;
  pvector<vector_int> _cube_map_pvs;

  pvector<MapStaticProp> _static_props;

  PT(RayTraceScene) _trace_scene;
  pvector<PT(RayTraceTriangleMesh)> _trace_meshes;

  SteamAudioSceneData _steam_audio_scene_data;
  CPTA_uchar _steam_audio_probe_data;
  CPTA_uchar _steam_audio_pathing_probe_data;

  LightDebugData _light_debug_data;

  friend class MapLightingEffect;
  friend class MapBuilder;
};

#include "mapData.I"

#endif // MAPDATA_H
