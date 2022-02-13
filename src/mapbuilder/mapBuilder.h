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
 *
 */
class MapGeomBase : public ReferenceCount {
public:
  PT(BoundingBox) _bounds;
  bool _is_mesh;
  bool _in_group;

  virtual bool overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const=0;
};

/**
 * A collection of nearby world polygons and mesh entities that should be
 * treated as a single unit (aka flattened together).  If vis is active,
 * the vis builder will assign the group to the set of area clusters that
 * it intersects with.
 */
struct MapGeomGroup {
public:
  PT(BoundingBox) bounds;
  pvector<MapGeomBase *> geoms;
  BitArray clusters;

  bool _can_see_sky;
};

/**
 * A single planar polygon of a MapMesh.
 */
class MapPoly : public MapGeomBase {
public:
  // The winding defines the vertices of the polygon and the plane that it
  // lies on.
  Winding _winding;

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

  // Where the Geom of the polygon lives in the output scene graph.
  // Needed to keep track of polys for the lightmapper.
  GeomNode *_geom_node = nullptr;
  int _geom_index = -1;

  // Original side ID in the .vmf file.
  int _side_id;

  virtual bool overlaps_box(const LPoint3 &box_center, const LVector3 &box_half) const override;
};

/**
 * Meshes are simply collections of polygons.  They are associated with an
 * entity.  Solids are turned into meshes with each side being a MapPoly.
 * Displacements are also turned into meshes, and each triangle of the
 * displacement becomes a MapPoly.
 */
class MapMesh : public MapGeomBase {
public:
  pvector<PT(MapPoly)> _polys;
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

  //void build_mesh_groups();
  void divide_meshes(const pvector<MapGeomBase *> &geoms, const LPoint3 &node_mins, const LPoint3 &node_maxs);

  void add_poly_to_geom_node(MapPoly *poly, GeomVertexData *vdata, GeomNode *geom_node);

private:
  void build_entity_polygons(int entity);

public:
  PT(MapFile) _source_map;
  MapBuildOptions _options;

  pvector<MapGeomGroup> _mesh_groups;

  pvector<PT(MapMesh)> _meshes;

  int _world_mesh_index;

  PT(MapData) _out_data;
  PT(PandaNode) _out_top;
  PT(MapRoot) _out_node;

  LPoint3 _scene_mins;
  LPoint3 _scene_maxs;
  PT(BoundingBox) _scene_bounds;
};

#include "mapBuilder.I"

#endif // MAPBUILDER_H
