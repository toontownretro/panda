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
};

/**
 *
 */
class MapPoly : public MapGeomBase {
public:
  // The winding defines the vertices of the polygon and the plane that it
  // lies on.
  Winding _winding;

  PT(Material) _material;
  LVector4 _texture_vecs[2];
  LVector4 _lightmap_vecs[2];
  LVecBase2i _lightmap_mins;
  LVecBase2i _lightmap_size;

  // Contents of the surface as specified in the material.  Examples are
  // skybox, clip, nodraw, etc.
  unsigned int _contents;

  // The lightmaps for the polygon.  Bounced and direct light are separated
  // to support "stationary" lights, which are rendered dynamically but have
  // baked indirect contribution.  There are 4 separate lightmaps for the
  // indirect and direct lighting, to support normal mapped radiosity.  There
  // is one lightmap for the flat non-normal mapped polygon, and 3 extra
  // lightmaps for each normal basis direction.
  PNMImage _indirect_light[NUM_LIGHTMAPS];
  PNMImage _direct_light[NUM_LIGHTMAPS];

  bool _vis_occluder;

  // Where the Geom of the polygon lives in the output scene graph.
  // Needed to keep track of polys for the lightmapper.
  GeomNode *_geom_node = nullptr;
  int _geom_index = -1;

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
  };

  MapBuilder(const MapBuildOptions &options);

  ErrorCode build();

  ErrorCode build_polygons();

  ErrorCode build_lighting();

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
