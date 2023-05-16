/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapBuilder.h
 * @author brian
 * @date 2021-07-06
 */

#ifndef MAPBUILDER_H
#define MAPBUILDER_H

#include "pandabase.h"
#include "mapBuildOptions.h"
#include "mapObjects.h"
#include "thread.h"
#include "winding.h"
#include "mathutil_misc.h"
#include "pnmImage.h"
#include "material.h"
#include "mapData.h"
#include "mapRoot.h"
#include "bitArray.h"
#include "pset.h"

#define NUM_LIGHTMAPS (NUM_BUMP_VECTS + 1)

/**
 * Base class for geometry generated from a brush face in the level file.
 *
 * Corresponds to a single Geom in the output scene graph.
 */
class MapGeom : public ReferenceCount {
public:
  MapGeom(int side_id, Material *mat, Texture *base_tex);

  INLINE void set_lightmap_size(const LVecBase2i &size) { _lightmap_size = size; }

  INLINE int get_side_id() const { return _side_id; }

  INLINE Material *get_material() const { return _material; }
  INLINE Texture *get_base_tex() const { return _base_tex; }

  INLINE void set_geom(GeomNode *node, int geom_index) { _geom_node = node; _geom_index = geom_index; }
  INLINE GeomNode *get_geom_node() const { return _geom_node; }
  INLINE int get_geom_index() const { return _geom_index; }

  void add_vertex_data(LPoint3 pos, LVector3 normal, LVecBase2 uv, LVecBase2 lightmap_uv);
  void add_vertex_data(LPoint3 pos, LVector3 normal, LVecBase2 uv, LVecBase2 lightmap_uv, PN_stdfloat alpha);
  void add_triangle(int v0, int v1, int v2);
  void add_index(int vertex) { _index.push_back(vertex); }

  INLINE bool has_normal() const { return !_normal.empty(); }
  INLINE bool has_alpha() const { return !_alpha.empty(); }
  INLINE bool has_index() const { return !_index.empty(); }

  INLINE int get_num_vertices() const {
    if (has_index()) {
      return (int)_index.size();
    } else {
      return (int)_pos.size();
    }
  }

  INLINE int get_num_vertex_rows() const {
    return (int)_pos.size();
  }

  INLINE LPoint3 get_pos(int n) const { return _pos[n]; }
  INLINE LVector3 get_normal(int n) const { return _normal[n]; }
  INLINE PN_stdfloat get_alpha(int n) const { return _alpha[n]; }
  INLINE LVecBase2 get_uv(int n) const { return _uv[n]; }
  INLINE LVecBase2 get_lightmap_uv(int n) const { return _lightmap_uv[n]; }
  INLINE int get_index(int n) const {
    if (!has_index()) {
      return n;
    } else {
      return _index[n];
    }
  }

  INLINE bool is_visible() const { return _visible; }

  bool get_winding(Winding &w) const;

  INLINE bool has_geom() const {
    return _geom_node != nullptr && _geom_index != -1;
  }

  INLINE bool is_empty() const {
    return _pos.empty();
  }

protected:
  PT(Material) _material;
  PT(Texture) _base_tex;

  // Where the Geom of the polygon lives in the output scene graph.
  // Needed to keep track of polys for the lightmapper.
  GeomNode *_geom_node = nullptr;
  int _geom_index = -1;

  // Original side ID in the .vmf file.
  int _side_id;

  // False if visibility system determines that the geom is
  // fully in solid-space.  In this case, the geom won't be
  // part of the scene graph.
  bool _visible;

  // Vertex data.
  pvector<LPoint3> _pos;
  pvector<LVector3> _normal;
  vector_stdfloat _alpha;
  pvector<LVecBase2> _uv;
  pvector<LVecBase2> _lightmap_uv;
  vector_int _index;

  LVecBase2i _lightmap_size;
};

/**
 *
 */
class MapGeomBase : public ReferenceCount {
public:
  PT(BoundingBox) _bounds;
  bool _is_mesh;
  bool _in_group;

  virtual bool overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const=0;
};

#if 0
/**
 * A single planar polygon of a MapMesh.
 */
class MapPoly : public MapGeomBase {
public:
  MapPoly() = default;

  // The winding defines the vertices of the polygon and the plane that it
  // lies on.
  Winding _winding;

  bool _sees_sky = false;
  bool _in_3d_skybox = false;
  pset<int> _leaves;

  pvector<LVector3> _normals;
  pvector<LVecBase2> _uvs;
  pvector<LVecBase2> _lightmap_uvs;
  vector_stdfloat _blends;

  PT(Material) _material;
  PT(Texture) _base_tex;

  LVecBase2i _lightmap_size;

  // Contents of the surface as specified in the material.  Examples are
  // skybox, clip, nodraw, etc.
  unsigned int _contents;

  bool _vis_occluder;
  bool _renderable;



  bool _visible;

  virtual bool overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const override;
};
#endif

/**
 * Meshes are simply collections of polygons.  They are associated with an
 * entity.  Solids are turned into meshes with each side being a MapPoly.
 * Displacements are also turned into meshes, and each triangle of the
 * displacement becomes a MapPoly.
 */
class MapMesh : public MapGeomBase {
public:
  MapMesh() = default;

  bool _3d_sky_mesh = false;

  pvector<PT(MapGeom)> _polys;
  // If true, the polygons of the mesh block visibility.  This only applies to
  // world meshes.
  //bool _vis_occluder;

  bool _in_mesh_group;

  int _entity;

  virtual bool overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const override;
};

class EXPCL_PANDA_MAPBUILDER MapBuilder {
PUBLISHED:
  enum ErrorCode {
    EC_ok,
    EC_unknown_error,
    EC_input_not_found,
    EC_input_invalid,
    EC_invalid_solid_side,
    EC_lightmap_failed,
    EC_steam_audio_failed,
  };

  MapBuilder(const MapBuildOptions &options);

  ErrorCode build();

  ErrorCode build_polygons();

  ErrorCode build_lighting();

  ErrorCode render_cube_maps();

  ErrorCode bake_steam_audio();

  void build_entity_physics(int entity, MapModel &model);

  void add_poly_to_geom_node(MapGeom *poly, GeomVertexData *vdata, GeomNode *geom_node);

  void build_overlays();

private:
  void build_entity_polygons(int entity);

  void r_collect_geoms(PandaNode *node, pvector<std::pair<CPT(Geom), CPT(RenderState) > > &geoms);

public:
  PT(MapFile) _source_map;
  MapBuildOptions _options;

  // This maps MapSide IDs to the list of MapPolys generated from
  // the side.
  typedef pmap<int, pvector<PT(MapGeom)>> SidePolys;
  SidePolys _side_polys;

  pvector<PT(MapMesh)> _meshes;
  // World polygons in the 3-D skybox.  Determined by the BSP visibility
  // builder.
  PT(MapMesh) _3d_sky_mesh;
  int _3d_sky_mesh_index;

  int _world_mesh_index;

  PT(MapData) _out_data;
  PT(PandaNode) _out_top;
  PT(MapRoot) _out_node;

  LPoint3 _scene_mins;
  LPoint3 _scene_maxs;
  PT(BoundingBox) _scene_bounds;

  // Extracted from the VisBuilder to place audio probes.
  pvector<LPoint3> _portal_centers;
};

#include "mapBuilder.I"

#endif // MAPBUILDER_H
