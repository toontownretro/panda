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
class MapPoly : public ReferenceCount {
public:
  // The winding defines the vertices of the polygon and the plane that it
  // lies on.
  Winding _winding;

  PT(Material) _material;
  LVector4 _texture_vecs[2];
  LVector4 _lightmap_vecs[2];

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
  // The set of area clusters that the polygon belongs to.  This is only used
  // for polygons that belong to the world.  All other meshes (and external
  // models) are placed into clusters at the model level.
  pset<int> _clusters;
};

/**
 * Meshes are simply collections of polygons.  They are associated with an
 * entity.  Solids are turned into meshes with each side being a MapPoly.
 * Displacements are also turned into meshes, and each triangle of the
 * displacement becomes a MapPoly.
 */
class MapMesh : public ReferenceCount {
public:
  pvector<PT(MapPoly)> _polys;
  PT(BoundingBox) _bounds;
  // If true, the polygons of the mesh block visibility.  This only applies to
  // world meshes.
  //bool _vis_occluder;

  // The set of area clusters that the mesh belongs to.  For the world mesh,
  // this is determined for each polygon.
  pset<int> _clusters;

  bool _in_mesh_group;
};

class EXPCL_PANDA_MAPBUILDER MapBuilder {
PUBLISHED:
  enum ErrorCode {
    EC_ok,
    EC_unknown_error,
    EC_input_not_found,
    EC_input_invalid,
    EC_invalid_solid_side,
  };

  MapBuilder(const MapBuildOptions &options);

  ErrorCode build();

  ErrorCode build_polygons();

  void add_poly_to_geom_node(MapPoly *poly, GeomVertexData *vdata, GeomNode *geom_node);

private:
  void build_entity_polygons(int entity);

public:
  PT(MapFile) _source_map;
  MapBuildOptions _options;

  pvector<PT(MapMesh)> _meshes;

  int _world_mesh_index;

  PT(MapData) _out_data;
  PT(PandaNode) _out_top;
  PT(MapRoot) _out_node;
};

#include "mapBuilder.I"

#endif // MAPBUILDER_H
