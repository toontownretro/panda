/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visBuilder.cxx
 * @author brian
 * @date 2021-07-08
 */

#include "visBuilder.h"
#include "mapBuilder.h"
#include "voxelSpace.h"
#include "visTile.h"
#include "clockObject.h"
#include "randomizer.h"
#include "threadManager.h"
//#include "ssemath.h"
//#include "rayTraceHitResult4.h"
#include "keyValues.h"
#include "kdTree.h"
#include "mathutil_misc.h"

#ifndef CPPPARSER
#include <array>
#endif

#include <stack>

#ifndef CPPPARSER
#include "rapid-profile.hpp"
#endif

NotifyCategoryDeclNoExport(visbuilder);
NotifyCategoryDef(visbuilder, "mapbuilder");

#define CheckBit( bitstring, bitNumber )	( (bitstring)[ ((bitNumber) >> 3) ] & ( 1 << ( (bitNumber) & 7 ) ) )
#define SetBit( bitstring, bitNumber )	( (bitstring)[ ((bitNumber) >> 3) ] |= ( 1 << ( (bitNumber) & 7 ) ) )
#define ClearBit( bitstring, bitNumber )	( (bitstring)[ ((bitNumber) >> 3) ] &= ~( 1 << ( (bitNumber) & 7 ) ) )

static int
count_bits(const unsigned char *bits, size_t num_bits) {
  int c = 0;
  for (size_t i = 0; i < num_bits; i++) {
    if (CheckBit(bits, i)) {
      c++;
    }
  }
  return c;
}

/**
 * Given a box and triangle that intersects the box, returns true if the box
 * is actually within the triangle and doesn't just border an edge.
 */
static bool
tri_box_check_edge(const LPoint3 &center, const LVector3 &half, const pvector<LPoint3> &v) {
  LPoint3 min = center - half;
  LPoint3 max = center + half;
  return true;
}

static void
fix_bounds(LPoint3 &mins, LPoint3 &maxs) {
  // Ensure that the bounds are not flat on any axis.
  for (int i = 0; i < 3; i++) {
    if (mins[i] == maxs[i]) {
      mins[i] -= 0.1;
      maxs[i] += 0.1;
    }
  }
}

#ifndef CPPPARSER
typedef std::array<LPoint3, 4> Quad;
#else
class Quad;
#endif


/**
 *
 */
VisBuilder::
VisBuilder(MapBuilder *builder) :
  _builder(builder)
{
}

/**
 *
 */
bool VisBuilder::
build() {
  visbuilder_cat.info()
    << "Vis start\n";

  _scene_mins = _builder->_scene_mins;
  _scene_maxs = _builder->_scene_maxs;
  _scene_bounds = _builder->_scene_bounds;

  visbuilder_cat.info()
    << "Scene bounds: mins " << _scene_mins << ", maxs " << _scene_maxs << "\n";

  const MapBuildOptions &opts = _builder->_options;

  LVector3 scene_vector = _scene_maxs - _scene_mins;

  int cells_x = (int)std::ceil(scene_vector[0] / opts._vis_voxel_size[0]) + 1;
  int cells_y = (int)std::ceil(scene_vector[1] / opts._vis_voxel_size[1]) + 1;
  int cells_z = (int)std::ceil(scene_vector[2] / opts._vis_voxel_size[2]) + 1;

  _voxels = VoxelSpace(opts._vis_voxel_size, LVecBase3i(cells_x, cells_y, cells_z), _scene_bounds);

  visbuilder_cat.info()
    << "Voxelizing scene\n";

  RayTrace::initialize();
  _occluder_scene = new RayTraceScene;
  _occluder_scene->set_build_quality(RayTraceScene::BUILD_QUALITY_HIGH);

  voxelize_scene();

  visbuilder_cat.info()
    << _voxels.get_num_solid_voxels() << " solid voxels, "
    << _voxels.get_num_voxels() - _voxels.get_num_solid_voxels()
    << " empty voxels\n";

  create_tiles();

  visbuilder_cat.info()
    << _vis_tiles.size() << " vis tiles\n";

  create_areas();

  visbuilder_cat.info()
    << _areas.size() << " areas\n";

  create_portals();

  visbuilder_cat.info()
    << _portals.size() << " portals\n";

  create_area_clusters();

  flood_entities();

  for (size_t i = 0; i < _area_clusters.size(); i++) {
    for (size_t j = 0; j < _area_clusters[i]->_portals.size(); j++) {
      _area_clusters[i]->_portals[j]->_id = (int)_cluster_portals.size();
      _cluster_portals.push_back(_area_clusters[i]->_portals[j]);
    }
  }

  simplify_area_clusters();

  // Fix up cluster ID's
  for (size_t i = 0; i < _area_clusters.size(); i++) {
    _area_clusters[i]->_id = i;
  }

  // We don't need our AABB tree anymore.
  _area_tree.clear();

  // We don't need tiles, the original areas, or original portals.
  _vis_tiles.clear();
  _areas.clear();
  _portals.clear();

  visbuilder_cat.info()
    << "Building final area cluster k-d tree...\n";

  // This is the tree that will be used at runtime to query the cluster(s) of
  // the camera and renderables.
  KDTree cluster_tree;
  for (size_t i = 0; i < _area_clusters.size(); i++) {
    const AreaCluster *cluster = _area_clusters[i];
    for (const AreaCluster::AreaBounds &ab : cluster->_cluster_boxes) {
      PT(BoundingBox) bbox = _voxels.get_voxel_bounds(ab._min_voxel, ab._max_voxel);
      cluster_tree.add_input(bbox->get_minq(), bbox->get_maxq(), (int)i);
    }
  }
  cluster_tree.build();
  visbuilder_cat.info()
    << "Area cluster tree is " << (PN_stdfloat)cluster_tree.get_memory_size() / 1000000.0f << " MB\n";
  visbuilder_cat.info()
    << cluster_tree.get_num_nodes() << " nodes, " << cluster_tree.get_num_leaves() << " leaves\n";
  _builder->_out_data->set_area_cluster_tree(std::move(cluster_tree));

  // Assign each mesh group created by the MapBuilder to the area clusters that
  // it intersects with.
  ThreadManager::run_threads_on_individual(
    "AssignMeshGroupClusters", _builder->_mesh_groups.size(), false,
    std::bind(&VisBuilder::find_mesh_group_clusters, this, std::placeholders::_1));

  build_pvs();

  return true;
}

/**
 * Assigns a mesh group to the area clusters that it intersects with.
 */
void VisBuilder::
find_mesh_group_clusters(int i) {
  MapGeomGroup *group = &_builder->_mesh_groups[i];

  // Traverse K-D tree to get the set of clusters.

  const KDTree *tree = _builder->_out_data->get_area_cluster_tree();

  std::stack<int> stack;
  stack.push(0);

  while (!stack.empty()) {
    int node_index = stack.top();
    stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = tree->get_node(node_index);

      LPlane node_plane(0, 0, 0, -node->dist);
      node_plane[node->axis] = 1;

      bool got_front = false;
      bool got_back = false;

      for (size_t i = 0; i < group->geoms.size(); i++) {
        MapPoly *poly = (MapPoly *)group->geoms[i];
        PlaneSide side = poly->_winding.get_plane_side(node_plane);
        if (side == PS_front) {
          got_front = true;

        } else if (side == PS_back) {
          got_back = true;

        } else {
          got_front = got_back = true;
        }

        if (got_front && got_back) {
          break;
        }
      }

      if (got_front) {
        stack.push(node->right_child);
      }
      if (got_back) {
        stack.push(node->left_child);
      }

    } else {
      // Hit a leaf.  Add cluster to mesh group.
      const KDTree::Leaf *leaf = tree->get_leaf(~node_index);
      if (leaf->value != -1) {
        group->clusters.set_bit(leaf->value);
      }
    }
  }
}

/**
 * Creates a voxel representation of all occluder geometry in the level.
 */
void VisBuilder::
voxelize_scene() {
  MapMesh *world_mesh = _builder->_meshes[_builder->_world_mesh_index];

  _occluder_trimesh = new RayTraceTriangleMesh;
  _occluder_trimesh->set_build_quality(RayTraceScene::BUILD_QUALITY_HIGH);

  // Go through each triangle and mark the voxels they overlap with as solid.
  ThreadManager::run_threads_on_individual(
    "VoxelizePolygons", world_mesh->_polys.size(), false,
    std::bind(&VisBuilder::voxelize_world_polygon, this, std::placeholders::_1));

  //for (size_t i = 0; i < world_mesh->_polys.size(); i++) {
  //  voxelize_world_polygon(i);
  //}

  _occluder_trimesh->build();

  _occluder_scene->add_geometry(_occluder_trimesh);
  _occluder_scene->update();
}

/**
 *
 */
void VisBuilder::
voxelize_world_polygon(int i) {
  MapMesh *world_mesh = _builder->_meshes[_builder->_world_mesh_index];
  MapPoly *poly = world_mesh->_polys[i];

  if (!poly->_vis_occluder) {
    // Polygon does not block visibility.
    return;
  }

  pvector<LPoint3> verts;
  verts.resize(3);

  LVecBase3 voxel_half = (_builder->_options._vis_voxel_size * 0.5f);

  Winding *w = &poly->_winding;
  LPlane plane = w->get_plane();
  // Push the winding a little bit back on the normal so we don't get solid
  // voxels in front of walls or on top of floors.
  // Push back half a voxel.
  //LVecBase3 offset = -w->get_plane().get_normal();
  //offset[0] *= _voxels._voxel_size[0] * 0.5f;
  //offset[1] *= _voxels._voxel_size[1] * 0.5f;
  //offset[2] *= _voxels._voxel_size[2] * 0.5f;
  //w->translate(offset);

  LPoint3 w_mins, w_maxs;
  w->get_bounds(w_mins, w_maxs);
  fix_bounds(w_mins, w_maxs);
  BoundingBox w_bounds(w_mins, w_maxs);
  if (w_bounds.is_empty()) {
    visbuilder_cat.error()
      << "Empty winding bounds " << w_mins << " " << w_maxs << "\n";
  }
  w_bounds.local_object();
  pvector<PT(BoundingBox)> voxels_bounds = _voxels.get_voxel_bounds_within(&w_bounds);

  for (size_t j = 1; j < (w->get_num_points() - 1); j++) {
    verts[0] = w->get_point(0);
    verts[1] = w->get_point(j);
    verts[2] = w->get_point(j + 1);

    ThreadManager::lock();
    _occluder_trimesh->add_triangle(verts[0], verts[1], verts[2]);
    ThreadManager::unlock();

    for (size_t k = 0; k < voxels_bounds.size(); k++) {
      BoundingBox *voxel_bounds = voxels_bounds[k];
      LPoint3 voxel_mid = voxel_bounds->get_approx_center();

      // If the distance from the voxel center to the polygon plane is
      // greater than or equal to the half size of the voxel, then the polygon
      // is sandwiched between two voxels, one on each side of the polygon.
      // Ignore the voxel that is in front of the polygon.
      if (plane.dist_to_plane(voxel_mid) >= voxel_half[0]) {
        continue;
      }

      // Nudge the voxel size a bit to account for floating-point imprecision.
      if (tri_box_overlap(voxel_mid, voxel_half + 0.01f, verts[0], verts[1], verts[2])) {
        if (tri_box_check_edge(voxel_mid, voxel_half, verts)) {
          // Mark voxel as solid.
          ThreadManager::lock();
          _voxels.set_voxel_type(_voxels.get_voxel_coord(voxel_mid), VoxelSpace::VT_solid);
          ThreadManager::unlock();
        }
      }
    }
  }

  //w->translate(-offset);
}

/**
 *
 */
void VisBuilder::
create_tiles() {
  LPoint3i tile_size = _builder->_options.get_vis_tile_size();

  // Calc total tiles.
  LPoint3i num_tiles(
    (int)std::ceil((float)_voxels.get_voxel_counts()[0] / (float)tile_size[0]),
    (int)std::ceil((float)_voxels.get_voxel_counts()[1] / (float)tile_size[1]),
    (int)std::ceil((float)_voxels.get_voxel_counts()[2] / (float)tile_size[2])
  );

  std::cout << "tile size: " << tile_size << "\n";
  std::cout << "voxel counts: " << _voxels.get_voxel_counts() << "\n";
  std::cout << "tile counts: " << num_tiles << "\n";

  LPoint3i from, to, voxel_coord;

  for (int x = 0; x < num_tiles[0]; x++) {
    from[0] = x * tile_size[0];
    to[0] = x * tile_size[0] + tile_size[0] - 1;

    // Clamp to bounds.
    if (to[0] >= _voxels.get_voxel_counts()[0]) {
      to[0] = _voxels.get_voxel_counts()[0] - 1;
    }

    for (int y = 0; y < num_tiles[1]; y++) {
      from[1] = y * tile_size[1];
      to[1] = y * tile_size[1] + tile_size[1] - 1;

      if (to[1] >= _voxels.get_voxel_counts()[1]) {
        to[1] = _voxels.get_voxel_counts()[1] - 1;
      }

      for (int z = 0; z < num_tiles[2]; z++) {
        from[2] = z * tile_size[2];
        to[2] = z * tile_size[2] + tile_size[2] - 1;

        if (to[2] >= _voxels.get_voxel_counts()[2]) {
          to[2] = _voxels.get_voxel_counts()[2] - 1;
        }

        PT(VisTile) tile = new VisTile;
        tile->_min_voxel = from;
        tile->_max_voxel = to;
        tile->_num_solid_voxels = 0;
        tile->_head_node = _voxels._solid_voxels.get_lowest_node_containing_box(
          LPoint3(from[0], from[1], from[2]), LPoint3(to[0], to[1], to[2]));

        // Add the number of solid voxels to the tile.
        for (voxel_coord[0] = from[0]; voxel_coord[0] <= to[0]; voxel_coord[0]++) {
          for (voxel_coord[1] = from[1]; voxel_coord[1] <= to[1]; voxel_coord[1]++) {
            for (voxel_coord[2] = from[2]; voxel_coord[2] <= to[2]; voxel_coord[2]++) {
              if (_voxels._solid_voxels.contains(voxel_coord, tile->_head_node)) {
                tile->_num_solid_voxels++;
              }
            }
          }
        }

        _vis_tiles.push_back(tile);
      }
    }
  }
}

/**
 *
 */
void VisBuilder::
create_areas() {
  _areas_created = 0;

  ThreadManager::run_threads_on_individual(
    "CreateInitialAreas", _vis_tiles.size(), false,
    std::bind(&VisBuilder::create_tile_areas, this, std::placeholders::_1));

  // Merge tile areas into single area list.  Done separately for threading.
  _areas.reserve(_areas_created);
  for (VisTile *tile : _vis_tiles) {
    _areas.insert(_areas.end(), tile->_areas.begin(), tile->_areas.end());
  }

  visbuilder_cat.info()
    << "Building area AABB tree...\n";
  for (size_t i = 0; i < _areas.size(); i++) {
    const Area *area = _areas[i];
    PT(BoundingBox) area_bounds = _voxels.get_voxel_bounds(area->_min_voxel, area->_max_voxel);
    _area_tree.add_leaf(
      area_bounds->get_minq(), area_bounds->get_maxq(), (int)i);
  }
  _area_tree.build();
  visbuilder_cat.info()
    << _area_tree.get_num_nodes() << " area tree nodes\n";
  //_area_tree.output(visbuilder_cat.info(false));
}

/**
 *
 */
void VisBuilder::
create_tile_areas(int i) {
  VisTile *tile = _vis_tiles[i];

  int expected_empty_voxels = tile->get_num_voxels() - tile->_num_solid_voxels;
  tile->_areas.reserve(4096);

  // If all of the tile if solid, don't generate an area.
  if (expected_empty_voxels == 0) {
    return;
  }

  // While there are no pending empty voxels left in the tile.
  while (expected_empty_voxels > 0) {
    // Take first seed voxel index.
    LPoint3i min_voxel_coord, max_voxel_coord;
    min_voxel_coord = max_voxel_coord = find_seed_point_in_tile(tile);

    // Expand the area until there is something that blocks the growth in
    // that direction, then continue with the other directions.
    test_tile_expansion(min_voxel_coord, max_voxel_coord, VoxelSpace::ND_front, tile);
    test_tile_expansion(min_voxel_coord, max_voxel_coord, VoxelSpace::ND_back, tile);
    test_tile_expansion(min_voxel_coord, max_voxel_coord, VoxelSpace::ND_right, tile);
    test_tile_expansion(min_voxel_coord, max_voxel_coord, VoxelSpace::ND_left, tile);
    test_tile_expansion(min_voxel_coord, max_voxel_coord, VoxelSpace::ND_up, tile);
    test_tile_expansion(min_voxel_coord, max_voxel_coord, VoxelSpace::ND_down, tile);

    // Calculate number of voxels in the area.
    int num_area_voxels = (max_voxel_coord[0] - min_voxel_coord[0] + 1) *
                          (max_voxel_coord[1] - min_voxel_coord[1] + 1) *
                          (max_voxel_coord[2] - min_voxel_coord[2] + 1);
    expected_empty_voxels -= num_area_voxels;

    // Create the area.
    PT(Area) area = new Area;
    area->_min_voxel = min_voxel_coord;
    area->_max_voxel = max_voxel_coord;
    tile->_areas.push_back(area);
    AtomicAdjust::inc(_areas_created);
  }
}

/**
 *
 */
void VisBuilder::
create_portals() {
  _total_portals = 0;

  ThreadManager::run_threads_on_individual(
    "CreateInitialPortals", _areas.size(), false,
    std::bind(&VisBuilder::create_area_portals, this, std::placeholders::_1));

  _portals.reserve(_total_portals);
  for (int i = 0; i < (int)_areas.size(); i++) {
    _portals.insert(_portals.end(), _areas[i]->_portals.begin(), _areas[i]->_portals.end());
  }
}

/**
 *
 */
void VisBuilder::
create_area_portals(int i) {
  Area *area = _areas[i];

  pvector<LPoint3i> surrounding_voxels;
  get_voxels_surrounding_region(area->_min_voxel, area->_max_voxel, surrounding_voxels);

  // All the area external voxels that already constitute a portal.
  pvector<LPoint3i> used_surrounding_voxels;
  used_surrounding_voxels.reserve(surrounding_voxels.size());

  pvector<LPoint3i> shared_voxels;
  //shared_voxels.reserve((area->_max_voxel[0] - area->_min_voxel[0]) * (area->_max_voxel[1] - area->_min_voxel[1]));
  pvector<LPoint3i> shared_voxels_other;
  //shared_voxels_other.reserve((area->_max_voxel[0] - area->_min_voxel[0]) * (area->_max_voxel[1] - area->_min_voxel[1]));
  area->_portals.reserve((area->_max_voxel[0] - area->_min_voxel[0]) * (area->_max_voxel[1] - area->_min_voxel[1]));

  for (const LPoint3i &voxel : surrounding_voxels) {
    // Check if the voxel was already considered for a portal.
    if (std::find(used_surrounding_voxels.begin(), used_surrounding_voxels.end(), voxel) != used_surrounding_voxels.end()) {
      continue;
    }

    int area_index;
    // Get the area that contains this surrounding voxel.
    get_area_from_voxel(voxel, area_index);

    if (area_index == i || area_index < 0) {
      // Same area or area not found.
      continue;
    }

    // Get the voxels inside the current area that surround the other area.
    shared_voxels.clear();
    get_shared_voxels(area, _areas[area_index], shared_voxels);

    // Now get the converse of above, the voxels inside the other area that
    // surround the current area.
    shared_voxels_other.clear();
    get_shared_voxels(_areas[area_index], area, shared_voxels_other);

    // Add the voxels to the list of already considered voxels so portals
    // don't repeat themselves.
    used_surrounding_voxels.insert(used_surrounding_voxels.end(), shared_voxels_other.begin(), shared_voxels_other.end());

    LPoint3i min, max;
    get_bounds_of_voxels(shared_voxels, min, max);

    LVector3 plane = -get_portal_facing_wall_plane(min, max, _areas[area_index]);

    LPoint3i half = (max - min) / 2;
    LPoint3i center = (min + max) / 2;

    LPoint3 origin(_voxels.get_voxel_center(
      LPoint3i(center[0] + (half[0] * plane[0]),
                center[1] + (half[1] * plane[1]),
                center[2] + (half[2] * plane[2]))
    ));

    PT(Portal) portal = new Portal;
    //if (visbuilder_cat.is_debug()) {
    //  visbuilder_cat.debug()
    //    << "add portal connecting " << i << " to " << area_index << ", direction " << plane << "\n";
    //}
    portal->_origin = origin;
    portal->_min_voxel = min;
    portal->_max_voxel = max;
    portal->_from_area = area;
    portal->_to_area = _areas[area_index];
    portal->_plane = LPlane(plane, origin);
    Quad q = portal->get_quad(_voxels._voxel_size, _scene_mins);
    portal->_winding.add_point(q[3]);
    portal->_winding.add_point(q[2]);
    portal->_winding.add_point(q[1]);
    portal->_winding.add_point(q[0]);
    portal->_plane = portal->_winding.get_plane();
    portal->_origin = portal->_winding.get_center();
    area->_portals.push_back(portal);
    AtomicAdjust::inc(_total_portals);
  }
}

/**
 * Groups together neighboring areas with minimal occlusion between each other.
 */
void VisBuilder::
create_area_clusters() {
  visbuilder_cat.info() <<
    "Building area clusters...\n";

  pvector<Area *> empty_areas;
  for (int i = 0; i < (int)_areas.size(); i++) {
    empty_areas.push_back(_areas[i]);
  }

  while (!empty_areas.empty()) {
    // Select the first area that has not yet been assigned to a group.
    Area *area = empty_areas.back();
    empty_areas.pop_back();

    PT(AreaCluster) cluster = new AreaCluster;
    cluster->_min_voxel.set(INT_MAX, INT_MAX, INT_MAX);
    cluster->_max_voxel.set(INT_MIN, INT_MIN, INT_MIN);
    cluster->add_area(area);
    int index = (int)_area_clusters.size();
    cluster->_id = index;
    _area_clusters.push_back(cluster);

    // Add it to an initial group by itself.
    area->_group = index;

    try_expand_area_group(cluster, empty_areas, index);
  }

  size_t end_num_areas = _areas.size();

  visbuilder_cat.info()
    << _area_clusters.size() << " area clusters\n";
}

/**
 * Recursively tags area clusters that are visible to an entity.  Removes
 * clusters that are not reachable by any entity.
 */
void VisBuilder::
flood_entities() {
  visbuilder_cat.info()
    << "----Flood Entities----\n";

  MapFile *src_map = _builder->_source_map;
  for (size_t i = 1; i < src_map->_entities.size(); i++) {
    MapEntitySrc *ent = src_map->_entities[i];
    auto origin_it = ent->_properties.find("origin");
    if (origin_it == ent->_properties.end()) {
      continue;
    }
    LPoint3 origin = KeyValues::to_3f(origin_it->second);
    if (origin == LPoint3(0)) {
      continue;
    }

    // So objects on floor are okay.
    origin[2] += 1;

    // Find the cluster of the entity.
    int leaf = _area_tree.get_leaf_containing_point(origin);
    if (leaf < 0) {
      //visbuilder_cat.warning()
      //  << "Entity " << i << " (" << ent->_class_name << ") is outside the world\n";
      continue;
    }
    int area_index = _area_tree.get_node(leaf)->value;

    Area *entity_area = _areas[area_index];
    assert(entity_area->_group >= 0 && !entity_area->_is_cluster);
    AreaCluster *entity_cluster = _area_clusters[entity_area->_group];

    // Flood outward from the cluster, marking neighboring clusters along the
    // way.
    std::stack<AreaCluster *> stack;
    stack.push(entity_cluster);
    std::stack<pvector<LPoint3>> path_stack;

    pvector<LPoint3> entity_path = { origin };
    path_stack.push(entity_path);

    while (!stack.empty()) {
      AreaCluster *cluster = stack.top();
      stack.pop();
      pvector<LPoint3> path = path_stack.top();
      path_stack.pop();

      if (cluster->_occupied) {
        continue;
      }

      cluster->_occupied = true;
      cluster->_occupied_path = path;

      for (Portal *portal : cluster->_portals) {
        assert(portal->_to_area->_is_cluster);
        AreaCluster *neighbor = (AreaCluster *)portal->_to_area;
        if (!neighbor->_occupied) {
          pvector<LPoint3> neighbor_path = path;
          neighbor_path.push_back(portal->_origin);
          path_stack.push(neighbor_path);
          stack.push(neighbor);
        }
      }
    }
  }

  int num_removed = 0;

  // Remove un-occupied clusters.
  for (auto it = _area_clusters.begin(); it != _area_clusters.end();) {
    AreaCluster *cluster = *it;
    if (cluster->_occupied) {
      ++it;

    } else {
      num_removed++;
      // Cluster is not occupied by an entity, therefore it is outside the
      // playable world.  Remove the cluster and any portals on neighboring
      // clusters that lead into this cluster.

      for (Portal *portal : cluster->_portals) {
        assert(portal->_to_area->_is_cluster);
        AreaCluster *neighbor = (AreaCluster *)portal->_to_area;
        for (auto pit = neighbor->_portals.begin(); pit != neighbor->_portals.end();) {
          Portal *neighbor_portal = *pit;
          if (neighbor_portal->_to_area != cluster) {
            ++pit;

          } else {
            pit = neighbor->_portals.erase(pit);
          }
        }
      }

      it = _area_clusters.erase(it);
    }
  }

  visbuilder_cat.info()
    << "Removed " << num_removed << " unoccupied area clusters\n";
  visbuilder_cat.info()
    << "New cluster count " << _area_clusters.size() << "\n";
}

/**
 * Reduces the number of boxes contained in each area cluster to the minimum
 * without changing the overall geometry of the cluster.  Works by recursively
 * flood-filling voxels contained within the boxes of the cluster.
 */
void VisBuilder::
simplify_area_clusters() {
  ThreadManager::run_threads_on_individual(
    "SimplifyAreaClusters", _area_clusters.size(), false,
    std::bind(&VisBuilder::simplify_area_cluster, this, std::placeholders::_1));
}

/**
 * Simplifies a single area cluster.
 */
void VisBuilder::
simplify_area_cluster(int i) {
  AreaCluster *cluster = _area_clusters[i];

  int num_empty_voxels = 0;
  for (const AreaCluster::AreaBounds &ab : cluster->_contained_areas) {
    num_empty_voxels += (ab._max_voxel[0] - ab._min_voxel[0] + 1) *
                        (ab._max_voxel[1] - ab._min_voxel[1] + 1) *
                        (ab._max_voxel[2] - ab._min_voxel[2] + 1);
  }

  assert(num_empty_voxels > 0);

  while (num_empty_voxels > 0) {
    AreaCluster::AreaBounds ab;
    ab._min_voxel = ab._max_voxel = cluster->get_area_seed_point();

    // Expand as far as we can in each direction until we hit another
    // cluster box or a different cluster.
    cluster->test_expansion(ab._min_voxel, ab._max_voxel, VoxelSpace::ND_front, this);
    cluster->test_expansion(ab._min_voxel, ab._max_voxel, VoxelSpace::ND_back, this);
    cluster->test_expansion(ab._min_voxel, ab._max_voxel, VoxelSpace::ND_right, this);
    cluster->test_expansion(ab._min_voxel, ab._max_voxel, VoxelSpace::ND_left, this);
    cluster->test_expansion(ab._min_voxel, ab._max_voxel, VoxelSpace::ND_up, this);
    cluster->test_expansion(ab._min_voxel, ab._max_voxel, VoxelSpace::ND_down, this);

    int num_box_voxels = (ab._max_voxel[0] - ab._min_voxel[0] + 1) *
                         (ab._max_voxel[1] - ab._min_voxel[1] + 1) *
                         (ab._max_voxel[2] - ab._min_voxel[2] + 1);
    num_empty_voxels -= num_box_voxels;

    cluster->_cluster_boxes.push_back(std::move(ab));
  }

  if (cluster->_cluster_boxes.size() > cluster->_contained_areas.size()) {
    // "Simplification" resulted in more boxes.  Revert to the original set.
    cluster->_cluster_boxes = cluster->_contained_areas;
  }
}

/**
 * Generates a potentially visible set for each area cluster.
 */
void VisBuilder::
build_pvs() {
  visbuilder_cat.info()
    << _cluster_portals.size() / 2 << " cluster portals\n";

  _portal_bytes = ((_cluster_portals.size() + 63) & ~63) >> 3;
  _portal_longs = _portal_bytes / sizeof(long);

  ThreadManager::run_threads_on_individual(
    "BasePortalVis", _cluster_portals.size(), false,
    std::bind(&VisBuilder::base_portal_vis, this, std::placeholders::_1));

  sort_portals();

  ThreadManager::run_threads_on_individual(
    "PortalFlow", _sorted_portals.size(), false,
    std::bind(&VisBuilder::portal_flow, this, std::placeholders::_1));
  //for (size_t i = 0; i < _sorted_portals.size(); i++) {
  //  std::cout << "Portal flow " << i << "\n";
  //  std::cout << "Might see " << _sorted_portals[i]->_num_might_see << "\n";
  //  portal_flow(i);
  //}

  ThreadManager::run_threads_on_individual(
    "FinalClusterPVS", _area_clusters.size(), false,
    std::bind(&VisBuilder::final_cluster_pvs, this, std::placeholders::_1));

  //visbuilder_cat.info()
  //  << "PVS:\n";
  //for (AreaCluster *cluster : _area_clusters) {
  // visbuilder_cat.info()
  //    << "Cluster " << cluster->_id << ": ";
  //  for (int cluster_id : cluster->_pvs) {
  //    visbuilder_cat.info(false)
  //      << cluster_id << " ";
  //  }
  //  visbuilder_cat.info(false)
  //    << "\n";
  //}

  // Store PVS data on the output map.
  for (size_t i = 0; i < _area_clusters.size(); i++) {
    AreaClusterPVS pvs;

    for (int cluster_id : _area_clusters[i]->_pvs) {
      pvs.add_visible_cluster(cluster_id);
    }

    // Assign mesh groups to the cluster.
    int mesh_group_index = 0;
    for (auto it = _builder->_mesh_groups.begin(); it != _builder->_mesh_groups.end(); ++it) {
      if (it->clusters.get_bit(_area_clusters[i]->_id)) {
        // Mesh group resides in this area cluster.
        pvs.set_mesh_group(mesh_group_index);
      }
      mesh_group_index++;
    }

    _builder->_out_data->add_cluster_pvs(std::move(pvs));
  }

  //ThreadManager::run_threads_on_individual(
  //  "PotentiallyVisibleSet", _area_clusters.size(), false,
  //  std::bind(&VisBuilder::build_cluster_pvs, this, std::placeholders::_1));

  //for (size_t i = 0; i < _area_clusters.size(); i++) {
  //  build_cluster_pvs(i);
  //}
}

/**
 * For each cluster, merges vis bits for each portal of cluster onto cluster.
 */
void VisBuilder::
final_cluster_pvs(int i) {
  AreaCluster *cluster = _area_clusters[i];

  unsigned char *portalvector = (unsigned char *)alloca(_portal_bytes);
  memset(portalvector, 0, _portal_bytes);

  // Merge all portal vis into portalvector for this cluster.
  for (Portal *portal : cluster->_portals) {
    for (size_t j = 0; j < _portal_longs; j++) {
      ((long *)portalvector)[j] |= ((long *)portal->_portal_vis)[j];
    }
    SetBit(portalvector, portal->_id);
  }

  // Count the cluster itself.
  cluster->_pvs.insert(cluster->_id);

  // Now count other visible clusters.
  for (size_t i = 0; i < _cluster_portals.size(); i++) {
    if (CheckBit(portalvector, _cluster_portals[i]->_id)) {
      cluster->_pvs.insert(((AreaCluster *)_cluster_portals[i]->_to_area)->_id);
    }
  }
}

/**
 * Sorts the portals from the least complex, so the later ones can reuse the
 * earlier information.
 */
void VisBuilder::
sort_portals() {
  _sorted_portals.insert(_sorted_portals.end(), _cluster_portals.begin(), _cluster_portals.end());
  std::sort(_sorted_portals.begin(), _sorted_portals.end(),
    [this](const Portal *a, const Portal *b) -> bool {
      return a->_num_might_see < b->_num_might_see;
    });
}

/**
 * First PVS pass.  For each portal, finds all the other portals that are at
 * all possible to see from that portal.  Speeds up the final PVS pass.
 */
void VisBuilder::
base_portal_vis(int i) {
  Portal *p = _cluster_portals[i];

  p->calc_radius();

  // Allocate memory for bitwise vis solutions for this portal.
  p->_portal_front = new unsigned char[_portal_bytes];
  memset(p->_portal_front, 0, _portal_bytes);
  p->_portal_flood = new unsigned char[_portal_bytes];
  memset(p->_portal_flood, 0, _portal_bytes);
  p->_portal_vis = new unsigned char[_portal_bytes];
  memset(p->_portal_vis, 0, _portal_bytes);
  p->_num_might_see = 0;

  // Test the portal against all of the other portals in the map.
  for (size_t j = 0; j < _cluster_portals.size(); j++) {
    if (j == i) {
      // Don't test against itself.
      continue;
    }

    Portal *tp = _cluster_portals[j];

    PortalWinding *w = &(tp->_winding);
    // Classify the other portal against the plane of this portal.
    PlaneSide other_side = w->get_plane_side(p->_plane);
    if (other_side == PS_back ||
        other_side == PS_on) {
      // Other portal lies on or is completely behind this portal.  There's
      // no way we can see it.
      continue;
    }

    w = &(p->_winding);
    // Now classify myself against the plane of the other portal.
    PlaneSide my_side = w->get_plane_side(tp->_plane);
    if (my_side == PS_front) {
      // This portal is completely in front of the other portal.  There's
      // no way we can see it.
      continue;
    }

    // Add current portal to given portal's list of visible portals.
    SetBit(p->_portal_front, tp->_id);
  }

  simple_flood(p, (AreaCluster *)p->_to_area);

  p->_num_might_see = count_bits(p->_portal_flood, _cluster_portals.size());
  //if (visbuilder_cat.is_debug()) {
  //  visbuilder_cat.debug()
  //    << "Portal " << i << ": " << p->_num_might_see << " might see\n";
  //}
}

/**
 *
 */
void VisBuilder::
simple_flood(Portal *src_portal, AreaCluster *cluster) {
  for (Portal *p : cluster->_portals) {
    int pnum = p->_id;
    if (!CheckBit(src_portal->_portal_front, pnum)) {
      continue;
    }
    if (CheckBit(src_portal->_portal_flood, pnum)) {
      continue;
    }
    SetBit(src_portal->_portal_flood, pnum);
    simple_flood(src_portal, (AreaCluster *)p->_to_area);
  }
}

/**
 *
 */
void VisBuilder::
portal_flow(int i) {
  Portal *p = _sorted_portals[i];
  AtomicAdjust::set(p->_status, Portal::S_working);
  PFThreadData data;
  data.base = p;
  data.visited = (unsigned char *)alloca(_portal_bytes);
  memset(data.visited, 0, _portal_bytes);
  data.pstack_head.portal = p;
  data.pstack_head.source = p->_winding;
  data.pstack_head.portal_plane = p->_plane;
  data.pstack_head.might_see = (unsigned char *)alloca(_portal_bytes);
  long *pf_long = (long *)p->_portal_flood;
  for (size_t j = 0; j < _portal_longs; j++) {
    ((long *)data.pstack_head.might_see)[j] = pf_long[j];
  }

  assert(count_bits(data.pstack_head.might_see, _cluster_portals.size()) == p->_num_might_see);

  recursive_cluster_flow((AreaCluster *)p->_to_area, &data, &data.pstack_head);

  AtomicAdjust::set(p->_status, Portal::S_done);

  //if (visbuilder_cat.is_debug()) {
  //  ThreadManager::lock();
  //  std::cerr
  //    << "Portal " << p->_id << ": " << count_bits(p->_portal_vis, _cluster_portals.size()) << " visible portals\n";
  //  ThreadManager::unlock();
  //}
}

/**
 *
 */
void VisBuilder::
recursive_cluster_flow(AreaCluster *cluster, PFThreadData *thread, PFStack *prevstack) {
  thread->c_chains++;
  PFStack stack;
  stack.might_see = (unsigned char *)alloca(_portal_bytes);
  prevstack->next = &stack;
  stack.next = nullptr;
  stack.cluster = cluster;
  stack.portal = nullptr;
  stack.num_separators[0] = 0;
  stack.num_separators[1] = 0;

  unsigned char *base_vis = thread->base->_portal_vis;
  LPoint3 base_origin = thread->base->_origin;
  LPlane base_plane = thread->pstack_head.portal_plane;
  PN_stdfloat base_radius = thread->base->_radius;

  long *might = (long *)stack.might_see;
  long *vis = (long *)thread->base->_portal_vis;

  unsigned char *prevmight = prevstack->might_see;
  long *prevmight_long = (long *)prevmight;

  long more;
  long *test;
  int pnum, n;
  Portal *p;
  size_t j, i;
  LPlane backplane;
  PN_stdfloat d;

  size_t num_portals = cluster->_portals.size();

  // Check all portals for flowing into other clusters.
  for (i = 0; i < num_portals; i++) {
    p = cluster->_portals[i];
    pnum = p->_id;

    if (!CheckBit(prevmight, pnum)) {
      continue; // Can't possibly see this portal.
    }

    //if (CheckBit(thread->visited, pnum)) {
    //  continue;
    //}

    //SetBit(thread->visited, pnum);

    // If the portal can't see anything we haven't already seen, skip it.
    if (p->_status == Portal::S_done) {
      test = (long *)p->_portal_vis;

    } else {
      test = (long *)p->_portal_flood;
    }

    more = 0;
    for (j = 0; j < _portal_longs; j++) {
      might[j] = prevmight_long[j] & test[j];
			more |= (might[j] & ~vis[j]);
    }

    if (!more && CheckBit(base_vis, pnum)) {
      // Can't see anything new.
      continue;
    }

    stack.portal_plane = p->_plane;
    backplane = -(stack.portal_plane);
    stack.portal = p;
    stack.next = nullptr;

    d = base_plane.dist_to_plane(p->_origin);
    if (d < -p->_radius) {
      continue;

    } else if (d > p->_radius) {
      stack.pass = p->_winding;

    } else {
      stack.pass = p->_winding.chop(base_plane);
      //stack.pass = chop_winding(&p->_winding, &stack, base_plane);
      if (stack.pass.is_empty()) {
        continue;
      }
    }

    d = p->_plane.dist_to_plane(base_origin);
    if (d > base_radius) {
      continue;

    } else if (d < -base_radius) {
      stack.source = prevstack->source;

    } else {
      stack.source = prevstack->source.chop(backplane);
      if (stack.source.is_empty()) {
        continue;
      }
      //stack.source = chop_winding(prevstack->source, &stack, backplane);
      //if (stack.source == nullptr || stack.source->is_empty()) {
      //  continue;
      //}
    }

    if (prevstack->pass.is_empty()) {
      // The second cluster can only be blocked if coplanar.
      // Mark the portal as visible
      //std::cout << "\tSet visible " << pnum << "\n";
      SetBit(base_vis, pnum);

      recursive_cluster_flow((AreaCluster *)p->_to_area, thread, &stack);
      continue;
    }

    if (stack.num_separators[0]) {
      for (n = 0; n < stack.num_separators[0]; n++) {
        stack.pass = stack.pass.chop(stack.separators[0][n]);
        if (stack.pass.is_empty()) {
          break;
        }
        //stack.pass = chop_winding(stack.pass, &stack, stack.separators[0][n]);
        //if (stack.pass == nullptr || stack.pass->is_empty()) {
        //  break; // Target is not visible.
        //}
      }
      if (n < stack.num_separators[0]) {
        continue;
      }
    } else {
      stack.pass = clip_to_seperators(prevstack->source, prevstack->pass, stack.pass, false, &stack);
    }

    if (stack.pass.is_empty()) {
      continue;
    }

    if (stack.num_separators[1]) {
      for (n = 0; n < stack.num_separators[1]; n++) {
        stack.pass = stack.pass.chop(stack.separators[1][n]);
        if (stack.pass.is_empty()) {
          break;
        }
        //stack.pass = chop_winding(stack.pass, &stack, stack.separators[1][n]);
        //if (stack.pass == nullptr || stack.pass->is_empty()) {
        //  break; // Target is not visible.
        //}
      }
    } else {
      stack.pass = clip_to_seperators(prevstack->pass, prevstack->source, stack.pass, true, &stack);
    }

    if (stack.pass.is_empty()) {
      continue;
    }

    //std::cout << "\tSet visible " << pnum << "\n";
    // Mark the portal as visible.
    SetBit(base_vis, pnum);

    // Flow through it for real.
    recursive_cluster_flow((AreaCluster *)p->_to_area, thread, &stack);
  }
}

#if 0
/**
 *
 */
Winding *VisBuilder::
chop_winding(Winding *w, PFStack *stack, const LPlane &plane) {
  Winding *dest = nullptr;
  for (int i = 0; i < 3; i++) {
    if (stack->windings[i].is_empty()) {
      dest = &stack->windings[i];
    }
  }
  assert(dest != nullptr);
  *dest = w->chop(plane);

  if (!dest->is_empty()) {
    size_t i = w - stack->windings;
    if (i >= 0 && i <= 2) {
      stack->windings[i].clear();
    }
  }

  return dest;
}
#endif

/**
 * Source, pass, and target are an ordering of portals.
 *
 * Generates separating planes candidates by taking two points from source and
 * one point from pass, and clips target by them.
 *
 * If target is totally clipped away, that portal cannot be seen through.
 *
 * Normal clip keeps target on the same side as pass, which is correct if the
 * order goes source, pass, target.  If the order goes pass, source, target,
 * then flip_clip should be set.
 */
PortalWinding VisBuilder::
clip_to_seperators(const PortalWinding &source, const PortalWinding &pass,
                   const PortalWinding &target, bool flip_clip,
                   PFStack *stack) {

  LVector3 v1, v2, normal;
  LPlane plane;
  PN_stdfloat length;

  PortalWinding new_target = target;

  // Check all combinations.
  for (int i = 0; i < source.get_num_points(); i++) {
    int l = (i + 1) % source.get_num_points();
    v1 = source.get_point(l) - source.get_point(i);

    // Find a vertex of pass that makes a plane that puts all of the
    // vertices of pass on the front side and all of the vertices of
    // source on the back side.
    int num_pass = pass.get_num_points();
    for (int ipass = 0; ipass < num_pass; ipass++) {
      v2 = pass.get_point(ipass) - source.get_point(i);

      normal = v1.cross(v2);

      // If points don't make a valid plane, skip it.
      length = normal.length_squared();
      if (length < 0.001f) {
        continue;
      }

      normal /= std::sqrt(length);

      plane.set(normal[0], normal[1], normal[2], -(pass.get_point(ipass).dot(normal)));

      // Find out which side of the generated separating plane has the
      // source portal.
      bool flip_test = false;
      int k;
      for (k = 0; k < source.get_num_points(); k++) {
        if (k == i || k == l) {
          continue;
        }
        PN_stdfloat d = plane.dist_to_plane(source.get_point(k));
        if (d < -0.001f) {
          // Source is on the negative side, so we want all pass and target
          // on the positive side.
          flip_test = false;
          break;

        } else if (d > 0.001f) {
          // Source is on the positive side, so we want all pass and target
          // on the negative side.
          flip_test = true;
          break;
        }
      }
      if (k == source.get_num_points()) {
        // Planar with source portal.
        continue;
      }

      // Flip the normal if the source portal is backwards.
      if (flip_test) {
        plane.flip();
      }

      // If all of the pass portal points are now on the positive side,
      // this is the separating plane.
      if (pass.get_plane_side(plane) != PS_front) {
        continue;
      }

      // Flip the normal if we want the back side.
      if (flip_clip) {
        plane.flip();
      }

      stack->separators[flip_clip][stack->num_separators[flip_clip]] = plane;
      if (++stack->num_separators[flip_clip] >= 64) {
        visbuilder_cat.error()
          << "MAX_SEPARATORS\n";
      }

      // Fast check first
      PN_stdfloat d = plane.dist_to_plane(stack->portal->_origin);
      // If completely at the back of the separator plane
      if (d < -stack->portal->_radius) {
        new_target.clear();
        return new_target;
      }
      // If completely on the front of the separator plane.
      if (d > stack->portal->_radius) {
        break;
      }

      // Clip target by the separating plane.
      new_target = new_target.chop(plane);
      if (new_target.is_empty()) {
        return new_target;
      }
      //target = chop_winding(target, stack, plane);
      //if (target->is_empty()) {
        // Target is not visible.
      //  return target;
      //}

      break;
    }
  }

  return new_target;
}

#define SSE_VIS_RAYS 0

/**
 * Attempts to expand the given area group with the neighbors of the given
 * area.
 */
void VisBuilder::
try_expand_area_group(AreaCluster *group, pvector<Area *> &empty_areas, int cluster_index) {
  Randomizer random;

  // To not check the same rejected neighbor over and over again.
  pset<Area *> rejected_neighbors;

  constexpr int num_rays = 5000;
  // The largest allowed size of a cluster on any AABB axis.
  // 256 hammer units, roughly 16 feet.  TODO: make this configurable.
  constexpr PN_stdfloat cluster_size_limit = 256.0f;
#if SSE_VIS_RAYS
  constexpr int rays_per_group = 4;
  constexpr int ray_groups = num_rays / rays_per_group;
  FourVectors start, end;
  LPoint3 starts[rays_per_group];
  LPoint3 ends[rays_per_group];
  u32x4 num_occluded_rays;
  fltx4 rays_per_group4 = ReplicateX4(rays_per_group);
  fltx4 num_rays4 = ReplicateX4(num_rays);
  fltx4 outgoing_portal_area4;
  fltx4 occlusion_threshold4 = ReplicateX4(16.0f);
  fltx4 occluded_ratio;
  fltx4 occlusion_value;
  RayTraceHitResult4 hit;
#else
  RayTraceHitResult hit;
#endif

  while (true) {
    // If the world-space size of the cluster has a reached the threshold
    // on any axis, this cluster is done.  It is an optimization to limit
    // the size of area clusters.  The bigger the area cluster, the more of
    // the world will be potentially visible to the cluster, which reduces
    // culling.
    LPoint3 curr_mins(1e24), curr_maxs(-1e24);
    _voxels.get_voxel_bounds(group->_min_voxel, group->_max_voxel, curr_mins, curr_maxs);
    LVector3 curr_size = curr_maxs - curr_mins;
    if (curr_size[0] >= cluster_size_limit ||
        curr_size[1] >= cluster_size_limit ||
        curr_size[2] >= cluster_size_limit) {
      // Size limit reached.  Cluster is complete.
      break;
    }

    // Get the current set of eligible neighbors for the cluster.
    pset<PT(Area)> neighbors;
    for (Portal *portal : group->_portals) {
      Area *neighbor = portal->_to_area;
      if (neighbor->_is_cluster ||
          neighbor->_group != -1 ||
          neighbor == group ||
          rejected_neighbors.find(neighbor) != rejected_neighbors.end()) {
        continue;
      }
      neighbors.insert(neighbor);
    }

    // If no eligible neighbors, the cluster is complete.
    if (neighbors.empty()) {
      break;
    }

    // Attempt to expand the cluster to each eligible neighbor.
    for (Area *neighbor : neighbors) {
      assert(!neighbor->_is_cluster);
      assert(neighbor->_group == -1);
      assert(neighbor != group);

      // Build up the lists of portals we will randomly cast rays between.
      pvector<Portal *> neighbor_portals;
      pvector<Portal *> my_portals;

      PN_stdfloat outgoing_portal_area = 0.0f;

      for (Portal *neighbor_portal : neighbor->_portals) {
        if (neighbor_portal->_to_area != group) {
          neighbor_portals.push_back(neighbor_portal);
          LVector3 outgoing_portal_size =
            LPoint3(neighbor_portal->_max_voxel[0], neighbor_portal->_max_voxel[1], neighbor_portal->_max_voxel[2]) -
            LPoint3(neighbor_portal->_min_voxel[0], neighbor_portal->_min_voxel[1], neighbor_portal->_min_voxel[2]);
          outgoing_portal_size[0] *= _builder->_options.get_vis_voxel_size()[0];
          outgoing_portal_size[1] *= _builder->_options.get_vis_voxel_size()[1];
          outgoing_portal_size[2] *= _builder->_options.get_vis_voxel_size()[2];

          outgoing_portal_area += neighbor_portal->_winding.get_area();
        }
      }

      for (Portal *my_portal : group->_portals) {
        if (my_portal->_to_area != neighbor) {
          my_portals.push_back(my_portal);
          LVector3 outgoing_portal_size =
            LPoint3(my_portal->_max_voxel[0], my_portal->_max_voxel[1], my_portal->_max_voxel[2]) -
            LPoint3(my_portal->_min_voxel[0], my_portal->_min_voxel[1], my_portal->_min_voxel[2]);
          outgoing_portal_size[0] *= _builder->_options.get_vis_voxel_size()[0];
          outgoing_portal_size[1] *= _builder->_options.get_vis_voxel_size()[1];
          outgoing_portal_size[2] *= _builder->_options.get_vis_voxel_size()[2];

          outgoing_portal_area += my_portal->_winding.get_area();
        }
      }

      //if (neighbor_portals.empty() || my_portals.empty()) {
        // No portals to cast rays between.  Neighbor can be added to cluster.
      //  group->add_area(neighbor);
      //  neighbor->_group = cluster_index;
      //  auto nit = std::find(empty_areas.begin(), empty_areas.end(), neighbor);
      //  assert(nit != empty_areas.end());
      //  empty_areas.erase(nit);
      //  continue;
      //}
#if SSE_VIS_RAYS
      outgoing_portal_area4 = ReplicateX4(outgoing_portal_area);

      num_occluded_rays = LoadZeroSIMD();
#else
      int num_occluded_rays = 0;
#endif

#if SSE_VIS_RAYS
      for (int i = 0; i < ray_groups; i++) {
        // Try to cast a ray to a random point within the outgoing portal to the
        // area we want to expand to.

        for (int j = 0; j < rays_per_group; j++) {
#else
        for (int i = 0; i < num_rays; i++) {
#endif

          LVector3i portal_size, min_voxel;
          if (!my_portals.empty()) {
            Portal *from_portal = my_portals[random.random_int(my_portals.size())];
            portal_size = from_portal->_max_voxel - from_portal->_min_voxel;
            min_voxel = from_portal->_min_voxel;

          } else {
            // If there's no portal to cast from, cast from a random point within the
            // cluster.
            int area_index = random.random_int(group->_contained_areas.size());
            portal_size = group->_contained_areas[area_index]._max_voxel - group->_contained_areas[area_index]._min_voxel;
            min_voxel = group->_contained_areas[area_index]._min_voxel;
          }
          PN_stdfloat goal_x = random.random_real(portal_size[0]) + min_voxel[0];
          PN_stdfloat goal_y = random.random_real(portal_size[1]) + min_voxel[1];
          PN_stdfloat goal_z = random.random_real(portal_size[2]) + min_voxel[2];
          LPoint3 a(goal_x * _voxels._voxel_size[0] + _scene_mins[0],
                    goal_y * _voxels._voxel_size[1] + _scene_mins[1],
                    goal_z * _voxels._voxel_size[2] + _scene_mins[2]);

          if (!neighbor_portals.empty()) {
            Portal *to_portal = neighbor_portals[random.random_int(neighbor_portals.size())];
            portal_size = to_portal->_max_voxel - to_portal->_min_voxel;
            min_voxel = to_portal->_min_voxel;

          } else {
            // If there's no portal to cast to, cast to a random point within
            // the neighbor.
            portal_size = neighbor->_max_voxel - neighbor->_min_voxel;
            min_voxel = neighbor->_min_voxel;
          }

          goal_x = random.random_real(portal_size[0]) + min_voxel[0];
          goal_y = random.random_real(portal_size[1]) + min_voxel[1];
          goal_z = random.random_real(portal_size[2]) + min_voxel[2];
          LPoint3 b(goal_x * _voxels._voxel_size[0] + _scene_mins[0],
                    goal_y * _voxels._voxel_size[1] + _scene_mins[1],
                    goal_z * _voxels._voxel_size[2] + _scene_mins[2]);

#if SSE_VIS_RAYS
          starts[j] = a;
          ends[j] = b;
#else
          hit = _occluder_scene->trace_line(a, b, BitMask32::all_on());
          if (hit.hit) {
            num_occluded_rays++;

            PN_stdfloat curr_value = ((PN_stdfloat)num_occluded_rays / (PN_stdfloat)num_rays) * outgoing_portal_area;
            if (curr_value > 48.0f*48.0f) {
              // If the occlusion value is already above the threshold, early out.
              break;
            }
          }
#endif
        }

#if SSE_VIS_RAYS
        start.LoadAndSwizzle(starts[0], starts[1], starts[2], starts[3]);
        end.LoadAndSwizzle(ends[0], ends[1], ends[2], ends[3]);

        _occluder_scene->trace_four_lines(start, end, Four_Ones, &hit);
        // If the ray hits, the value of CmpLtSIMD is -1, otherwise 0, so use SubSIMD to add a ray.
        num_occluded_rays = SubSIMD(num_occluded_rays, CmpLtSIMD(hit.hit_fraction, Four_Ones));
      }



      occluded_ratio = DivSIMD(num_occluded_rays, num_rays4);
      occlusion_value = MulSIMD(occluded_ratio, outgoing_portal_area4);

      //PN_stdfloat occluded_ratio = (PN_stdfloat)num_occluded_rays / (PN_stdfloat)num_rays;
      //PN_stdfloat occlusion_value = occluded_ratio * outgoing_portal_area;

      if ((SubFloat(occlusion_value, 0) + SubFloat(occlusion_value, 1) + SubFloat(occlusion_value, 2) + SubFloat(occlusion_value, 3))
          > 16.0f) {
        // Reject this neighbor from the cluster.
        rejected_neighbors.insert(neighbor);

      } else {
        // Expansion is valid!
        group->add_area(neighbor);
        neighbor->_group = cluster_index;
        auto nit = std::find(empty_areas.begin(), empty_areas.end(), neighbor);
        assert(nit != empty_areas.end());
        empty_areas.erase(nit);
      }
    }
#else
      PN_stdfloat occluded_ratio = (PN_stdfloat)num_occluded_rays / (PN_stdfloat)num_rays;
      PN_stdfloat occlusion_value = occluded_ratio * outgoing_portal_area;
      if (occlusion_value > 48.0f*48.0f) {
        // Reject this neighbor from the cluster.
        rejected_neighbors.insert(neighbor);

      } else {
        // Expansion is valid!
        group->add_area(neighbor);
        neighbor->_group = cluster_index;
        auto nit = std::find(empty_areas.begin(), empty_areas.end(), neighbor);
        assert(nit != empty_areas.end());
        empty_areas.erase(nit);
      }
    }
#endif
  }
}

/**
 * Computes the minimum and maximum extents of the given list of voxels.
 */
void VisBuilder::
get_bounds_of_voxels(const pvector<LPoint3i> &voxels, LPoint3i &min, LPoint3i &max) const {
  min.set(INT_MAX, INT_MAX, INT_MAX);
  max.set(INT_MIN, INT_MIN, INT_MIN);

  for (const LPoint3i &voxel : voxels) {
    min[0] = std::min(voxel[0], min[0]);
    min[1] = std::min(voxel[1], min[1]);
    min[2] = std::min(voxel[2], min[2]);
    max[0] = std::max(voxel[0], max[0]);
    max[1] = std::max(voxel[1], max[1]);
    max[2] = std::max(voxel[2], max[2]);
  }
}

/**
 * Fills up a list of voxels surrounding the indicated region of voxels.  If
 * solid_only is true, only solid voxels surrounding the region are returned.
 * Otherwise, both empty and solid voxels are returned.
 */
void VisBuilder::
get_voxels_surrounding_region(const LPoint3i &min, const LPoint3i &max,
                              pvector<LPoint3i> &voxels, bool solid_only) const {
  LPoint3i curr_voxel, from, to;

  // For each face of the area, find empty neighboring voxels.
  for (int face = 0; face < 6; face++) {
    from = min;
    to = max;

    switch (face) {
    case 0:
      // right
      from[0] = to[0] = (max[0] + 1);
      break;
    case 1:
      // left
      from[0] = to[0] = (min[0] - 1);
      break;
    case 2:
      // forward
      from[1] = to[1] = (max[1] + 1);
      break;
    case 3:
      // back
      from[1] = to[1] = (min[1] - 1);
      break;
    case 4:
      // up
      from[2] = to[2] = (max[2] + 1);
      break;
    case 5:
      // down
      from[2] = to[2] = (min[2] - 1);
      break;
    }

    for (int x = from[0]; x <= to[0]; x++) {
      for (int y = from[1]; y <= to[1]; y++) {
        for (int z = from[2]; z <= to[2]; z++) {
          curr_voxel.set(x, y, z);
          if (_voxels.is_valid_voxel_coord(curr_voxel)) {
            if (solid_only) {
              if (_voxels.get_voxel_type(curr_voxel) == VoxelSpace::VT_solid) {
                voxels.push_back(curr_voxel);
              }
            } else {
              voxels.push_back(curr_voxel);
            }
          }
        }
      }
    }
  }
}

/**
 * Finds the area that contains the indicated voxel.
 */
void VisBuilder::
get_area_from_voxel(const LPoint3i &voxel, int &area, int start) const {
  LPoint3 center = _voxels.get_voxel_center(voxel);
  int node = _area_tree.get_leaf_containing_point(center);
  if (node < 0) {
    area = -1;
  } else {
    area = _area_tree.get_node(node)->value;
  }
}

/**
 * Finds the voxels surrounding B that are contained in A.
 */
void VisBuilder::
get_shared_voxels(const Area *a, const Area *b, pvector<LPoint3i> &voxels) const {
  pvector<LPoint3i> surrounding;
  get_voxels_surrounding_region(b->_min_voxel, b->_max_voxel, surrounding);
  for (const LPoint3i &p : surrounding) {
    if (a->contains_voxel(p)) {
      // This voxel is part of A and is surrounding B.
      voxels.push_back(p);
    }
  }
}

/**
 * Returns the normal of the plane facing the voxel inside area that neighbors
 * the given range of voxels.
 */
LVector3 VisBuilder::
get_portal_facing_wall_plane(const LPoint3i &min, const LPoint3i &max, const Area *area) const {
  LPoint3i new_point;

  // Choose a neighbor voxel in each direction and see if it's inside area.
  // If it is, return the direction of the neighbor.

  // Test to the right.
  new_point = min;
  new_point[0]++;
  if (area->contains_voxel(new_point)) {
    return LVector3::right();
  }

  // Test to the left.
  new_point = min;
  new_point[0]--;
  if (area->contains_voxel(new_point)) {
    return LVector3::left();
  }

  // Test forward.
  new_point = min;
  new_point[1]++;
  if (area->contains_voxel(new_point)) {
    return LVector3::forward();
  }

  // Test backward.
  new_point = min;
  new_point[1]--;
  if (area->contains_voxel(new_point)) {
    return LVector3::back();
  }

  // Test up.
  new_point = min;
  new_point[2]++;
  if (area->contains_voxel(new_point)) {
    return LVector3::up();
  }

  // Test down
  new_point = min;
  new_point[2]--;
  if (area->contains_voxel(new_point)) {
    return LVector3::down();
  }

  visbuilder_cat.error()
    << "Wrong portal shared areas.\n";
  return LVector3::forward();
}

/**
 * Returns the first empty voxel inside the indicated vis tile.
 */
LPoint3i VisBuilder::
find_seed_point_in_tile(VisTile *tile) const {
  LPoint3i voxel;
  for (voxel[0] = tile->_min_voxel[0]; voxel[0] <= tile->_max_voxel[0]; voxel[0]++) {
    for (voxel[1] = tile->_min_voxel[1]; voxel[1] <= tile->_max_voxel[1]; voxel[1]++) {
      for (voxel[2] = tile->_min_voxel[2]; voxel[2] <= tile->_max_voxel[2]; voxel[2]++) {
        if (!_voxels._solid_voxels.contains(voxel, tile->_head_node) &&
            !tile->contains_voxel(voxel)) {
          return voxel;
        }
      }
    }
  }

  visbuilder_cat.error()
    << "Vis tile has no empty voxels\n";
  return voxel;
}

/**
 * Expands the given voxel bounds in the indicated direction until a collision
 * with a solid voxel occurs.
 */
void VisBuilder::
test_tile_expansion(LPoint3i &min_voxel, LPoint3i &max_voxel,
                    VoxelSpace::NeighborDirection direction,
                    VisTile *tile) const {
  LPoint3i offset(0);
  bool positive_dir = false;

  // Determine direction to expand.
  switch (direction) {
  case VoxelSpace::ND_front:
    offset[1] = 1;
    positive_dir = true;
    break;
  case VoxelSpace::ND_back:
    offset[1] = -1;
    positive_dir = false;
    break;
  case VoxelSpace::ND_right:
    offset[0] = 1;
    positive_dir = true;
    break;
  case VoxelSpace::ND_left:
    offset[0] = -1;
    positive_dir = false;
    break;
  case VoxelSpace::ND_up:
    offset[2] = 1;
    positive_dir = true;
    break;
  case VoxelSpace::ND_down:
    offset[2] = -1;
    positive_dir = false;
    break;
  }

  LPoint3i last_valid_max_voxel, last_valid_min_voxel;
  bool valid_new_bbox = true;

  // Expand AABB until it is not valid (collides w/ a solid voxel).
  while (valid_new_bbox) {
    last_valid_max_voxel = max_voxel;
    last_valid_min_voxel = min_voxel;

    // Determine where to expand from.
    if (positive_dir) {
      max_voxel += offset;
    } else {
      min_voxel += offset;
    }

    // Don't let the box be bigger than the setting.
    const LVecBase3 &max_cell_size = _builder->_options.get_vis_max_cell_size();
    LVector3i size = max_voxel - min_voxel;

    if (size[0] > max_cell_size[0] || size[1] > max_cell_size[1] || size[2] > max_cell_size[2]) {
      valid_new_bbox = false;
    } else {
      bool hit_solid;
      valid_new_bbox = try_new_bbox(min_voxel, max_voxel, direction, tile, hit_solid);
      //if ((!valid_new_bbox) && hit_solid) {
      //  // Allow the box to expand into one solid voxel.
      //  return;
      //}
    }
  }

  max_voxel = last_valid_max_voxel;
  min_voxel = last_valid_min_voxel;
}

/**
 * Returns true if the indicated voxel bounds are completely empty and do not
 * exceed the overall limits of the voxel space.
 */
bool VisBuilder::
try_new_bbox(LPoint3i &min_voxel, LPoint3i &max_voxel, VoxelSpace::NeighborDirection dir,
             VisTile *tile, bool &hit_solid) const {
  LPoint3i curr_voxel;
  LPoint3i from_voxel, to_voxel;

  from_voxel = min_voxel;
  to_voxel = max_voxel;

  // Determine face of the box that is expanding.
  switch (dir) {
  case VoxelSpace::ND_front:
    from_voxel[1] = to_voxel[1] = max_voxel[1];
    break;
  case VoxelSpace::ND_back:
    from_voxel[1] = to_voxel[1] = min_voxel[1];
    break;
  case VoxelSpace::ND_right:
    from_voxel[0] = to_voxel[0] = max_voxel[0];
    break;
  case VoxelSpace::ND_left:
    from_voxel[0] = to_voxel[0] = min_voxel[0];
    break;
  case VoxelSpace::ND_up:
    from_voxel[2] = to_voxel[2] = max_voxel[2];
    break;
  case VoxelSpace::ND_down:
    from_voxel[2] = to_voxel[2] = min_voxel[2];
    break;
  }

  hit_solid = false;

  // Check if the new expanded range goes outside of the tile.
  if (from_voxel[0] < tile->_min_voxel[0] || to_voxel[0] > tile->_max_voxel[0] ||
      from_voxel[1] < tile->_min_voxel[1] || to_voxel[1] > tile->_max_voxel[1] ||
      from_voxel[2] < tile->_min_voxel[2] || to_voxel[2] > tile->_max_voxel[2]) {
    return false;
  }

  // Create all the voxel points for that expanding face and check that they
  // are all empty.
  for (int x = from_voxel[0]; x <= to_voxel[0]; x++) {
    for (int y = from_voxel[1]; y <= to_voxel[1]; y++) {
      for (int z = from_voxel[2]; z <= to_voxel[2]; z++) {
        LPoint3i voxel(x, y, z);

        if (!_voxels.is_valid_voxel_coord(voxel)) {
          return false;

        } else if (tile->contains_voxel(voxel)) {
          return false;

        } else if (_voxels._solid_voxels.contains(voxel, tile->_head_node)) {
          hit_solid = true;
          return false;
        }
      }
    }
  }

  return true;
}
