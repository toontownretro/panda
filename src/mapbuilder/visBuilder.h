/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visBuilder.h
 * @author brian
 * @date 2021-07-08
 */

#ifndef VISBUILDER_H
#define VISBUILDER_H

#include "pandabase.h"
#include "luse.h"
#include "voxelSpace.h"
#include "visTile.h"
#include "pvector.h"
#include "area.h"
#include "portal.h"
#include "aabbTree.h"
#include "atomicAdjust.h"
#include "rayTrace.h"
#include "rayTraceScene.h"
#include "rayTraceTriangleMesh.h"
#include "vector_uchar.h"
#include "mapBuilder.h"
#include "bitArray.h"

class PFStack {
public:
  PFStack() {
    next = nullptr;
    cluster = nullptr;
    portal = nullptr;
    //source = pass = nullptr;
    might_see = nullptr;
    num_separators[0] = num_separators[1] = 0;
  }

  unsigned char *might_see;
  PFStack *next;
  AreaCluster *cluster;
  Portal *portal;
  PortalWinding source, pass;

  //Winding windings[3];

  LPlane portal_plane;

  LPlane separators[2][64];
  int num_separators[2];
};

class PFThreadData {
public:
  PFThreadData() {
    base = nullptr;
    visited = nullptr;
    c_chains = 0;
  }

  Portal *base;
  int c_chains;
  unsigned char *visited;
  PFStack pstack_head;
};

/**
 * Builds visibility information into the level.
 *
 * Algorithm based on the Janua occlusion engine https://github.com/gigc/Janua.
 */
class EXPCL_PANDA_MAPBUILDER VisBuilder {
public:
  VisBuilder(MapBuilder *builder);

  bool build();

  void voxelize_scene();

  void create_tiles();

  void create_areas();
  void create_portals();

  void create_area_clusters();

  void flood_entities();

  void simplify_area_clusters();

  void build_pvs();

private:

  void find_mesh_group_clusters(int i);

  void final_cluster_pvs(int i);

  //Winding *chop_winding(Winding *w, PFStack *stack, const LPlane &plane);

  PortalWinding clip_to_seperators(const PortalWinding &source, const PortalWinding &pass,
                                   const PortalWinding &target, bool flip_clip, PFStack *stack);

  void sort_portals();

  void portal_flow(int i);
  void recursive_cluster_flow(AreaCluster *cluster, PFThreadData *data, PFStack *stack);

  void base_portal_vis(int i);
  void simple_flood(Portal *portal, AreaCluster *cluster);

  void simplify_area_cluster(int i);

  void voxelize_world_polygon(int polygon);
  void create_tile_areas(int tile);
  void create_area_portals(int area);

  void try_expand_area_group(AreaCluster *cluster, pvector<Area *> &empty_areas, int cluster_index);

  LPoint3i find_seed_point_in_tile(VisTile *tile) const;

  void test_tile_expansion(LPoint3i &min_voxel, LPoint3i &max_voxel,
                           VoxelSpace::NeighborDirection direction,
                           VisTile *tile) const;
  bool try_new_bbox(LPoint3i &min_voxel, LPoint3i &max_voxel,
                    VoxelSpace::NeighborDirection direction,
                    VisTile *tile, bool &hit_solid) const;

  void get_voxels_surrounding_region(const LPoint3i &min, const LPoint3i &max,
                                     pvector<LPoint3i> &voxels, bool solid_only = false) const;
  void get_area_from_voxel(const LPoint3i &voxel, int &area, int start = 0) const;
  void get_shared_voxels(const Area *a, const Area *b, pvector<LPoint3i> &voxels) const;
  void get_bounds_of_voxels(const pvector<LPoint3i> &voxels, LPoint3i &mins, LPoint3i &maxs) const;
  LVector3 get_portal_facing_wall_plane(const LPoint3i &min, const LPoint3i &max, const Area *area) const;

public:
  MapBuilder *_builder;

  LPoint3 _scene_mins;
  LPoint3 _scene_maxs;
  PT(BoundingBox) _scene_bounds;

  AtomicAdjust::Integer _areas_created;
  AtomicAdjust::Integer _total_portals;

  VoxelSpace _voxels;

  pvector<PT(VisTile)> _vis_tiles;
  pvector<PT(Area)> _areas;
  pvector<PT(Portal)> _portals;

  // Simplification of area/portal graph.  Multiple areas are combined into a
  // single cluster based on amount of occlusion between neighboring areas.
  pvector<PT(AreaCluster)> _area_clusters;
  pvector<PT(Portal)> _cluster_portals;
  pvector<Portal *> _sorted_portals;

  // Spatial structure to quickly query the area that contains a voxel.
  AABBTreeInt _area_tree;

  // For ray tracing against occluder triangles.
  PT(RayTraceScene) _occluder_scene;
  PT(RayTraceTriangleMesh) _occluder_trimesh;

  size_t _portal_bytes;
  size_t _portal_longs;
};

#include "visBuilder.I"

#endif // VISBUILDER_H
