/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file area.cxx
 * @author brian
 * @date 2021-07-13
 */

#include "area.h"
#include "aabbTree.h"
#include "visBuilder.h"

/**
 *
 */
Area::
Area() {
  _group = -1;
  _is_cluster = false;
}

/**
 *
 */
AreaCluster::
AreaCluster() :
  Area()
{
  _is_cluster = true;
  _occupied = false;
}

/**
 * Adds an area into the cluster.  The bounds of the cluster are expanded to
 * include the given area, coincident portals between the cluster and
 * area are removed, and outgoing portals of the area are redirected to the
 * cluster.
 */
void AreaCluster::
add_area(Area *other) {
  AreaBounds ab;
  ab._min_voxel = other->_min_voxel;
  ab._max_voxel = other->_max_voxel;
  _contained_areas.push_back(std::move(ab));

  _min_voxel[0] = std::min(other->_min_voxel[0], _min_voxel[0]);
  _min_voxel[1] = std::min(other->_min_voxel[1], _min_voxel[1]);
  _min_voxel[2] = std::min(other->_min_voxel[2], _min_voxel[2]);
  _max_voxel[0] = std::max(other->_max_voxel[0], _max_voxel[0]);
  _max_voxel[1] = std::max(other->_max_voxel[1], _max_voxel[1]);
  _max_voxel[2] = std::max(other->_max_voxel[2], _max_voxel[2]);

  for (auto it = _portals.begin(); it != _portals.end();) {
    Portal *portal = (*it);
    if (portal->_to_area == other) {
      // Delete the portal on the cluster that leads into the area we are
      // adding to the cluster.
      it = _portals.erase(it);
    } else {
      ++it;
    }
  }

  // Now add the portals from the other area onto the cluster that lead into
  // other areas.
  for (auto it = other->_portals.begin(); it != other->_portals.end(); ++it) {
    Portal *portal = *it;

    if (portal->_to_area == this) {
      // Ignore portals that lead into ourselves.
      continue;
    }

    // Create a duplicate of the portal from the other area, except make it
    // originate from this area.
    PT(Portal) new_portal = new Portal;
    new_portal->_from_area = this;
    new_portal->_to_area = portal->_to_area;
    new_portal->_min_voxel = portal->_min_voxel;
    new_portal->_max_voxel = portal->_max_voxel;
    new_portal->_origin = portal->_origin;
    new_portal->_plane = portal->_plane;
    new_portal->_winding = portal->_winding;
    _portals.push_back(new_portal);

    // Redirect the coincident portal on the neighbor to ourselves.
    for (Portal *neighbor_portal : portal->_to_area->_portals) {
      if (neighbor_portal->_to_area == other) {
        neighbor_portal->_to_area = this;
      }
    }
  }
}

/**
 * Returns the coordinate of the first voxel that is contained in the cluster's
 * areas that isn't contained in the cluster's boxes.
 */
LPoint3i AreaCluster::
get_area_seed_point() const {
  for (const AreaBounds &ab : _contained_areas) {
    LPoint3i voxel;
    for (voxel[0] = ab._min_voxel[0]; voxel[0] <= ab._max_voxel[0]; voxel[0]++) {
      for (voxel[1] = ab._min_voxel[1]; voxel[1] <= ab._max_voxel[1]; voxel[1]++) {
        for (voxel[2] = ab._min_voxel[2]; voxel[2] <= ab._max_voxel[2]; voxel[2]++) {
          if (!boxes_contain_voxel(voxel)) {
            return voxel;
          }
        }
      }
    }
  }
  assert(false);
}

/**
 * Returns true if any of the cluster's boxes contain the indicated voxel.
 */
bool AreaCluster::
boxes_contain_voxel(const LPoint3i &voxel) const {
  for (const AreaBounds &ab : _cluster_boxes) {
    if (voxel[0] >= ab._min_voxel[0] && voxel[0] <= ab._max_voxel[0] &&
        voxel[1] >= ab._min_voxel[1] && voxel[1] <= ab._max_voxel[1] &&
        voxel[2] >= ab._min_voxel[2] && voxel[2] <= ab._max_voxel[2]) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if the indicated box intersects any of the cluster's existing
 * boxes.
 */
bool AreaCluster::
box_intersects_existing_boxes(const LPoint3i &min, const LPoint3i &max) const {
  for (const AreaBounds &ab : _cluster_boxes) {
    if (min[0] <= ab._max_voxel[0] || max[0] >= ab._min_voxel[0] ||
        min[1] <= ab._max_voxel[1] || max[1] >= ab._min_voxel[1] ||
        min[2] <= ab._max_voxel[2] || max[2] >= ab._min_voxel[2]) {
      return true;
    }
  }
  return false;
}

/**
 * Expands the indicated box in the given direction until something stops it.
 * Things stopping it would be hitting another box in the cluster or another
 * cluster.
 */
void AreaCluster::
test_expansion(LPoint3i &min, LPoint3i &max, VoxelSpace::NeighborDirection dir,
               VisBuilder *vis) const {
  LPoint3i offset(0);
  bool positive_dir = false;

  // Determine direction to expand.
  switch (dir) {
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

  while (true) {
    last_valid_max_voxel = max;
    last_valid_min_voxel = min;

    // Determine where to expand from.
    if (positive_dir) {
      max += offset;
    } else {
      min += offset;
    }

    if (!test_box(min, max, dir, vis)) {
      break;
    }
  }

  min = last_valid_min_voxel;
  max = last_valid_max_voxel;
}

/**
 *
 */
bool AreaCluster::
test_box(LPoint3i &min_voxel, LPoint3i &max_voxel, VoxelSpace::NeighborDirection dir,
         VisBuilder *vis) const {
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

  // Create all the voxel points for that expanding face and check that they
  // are all valid and within the cluster.
  for (int x = from_voxel[0]; x <= to_voxel[0]; x++) {
    for (int y = from_voxel[1]; y <= to_voxel[1]; y++) {
      for (int z = from_voxel[2]; z <= to_voxel[2]; z++) {
        LPoint3i voxel(x, y, z);

        if (!vis->_voxels.is_valid_voxel_coord(voxel)) {
          return false;
        }

        if (boxes_contain_voxel(voxel)) {
          // Voxel is in an existing box of this cluster.
          return false;
        }

        int leaf = vis->_area_tree.get_leaf_containing_point(vis->_voxels.get_voxel_center(voxel));
        if (leaf < 0) {
          return false;
        }
        int area_index = vis->_area_tree.get_node(leaf)->value;
        int cluster_id = vis->_areas[area_index]->_group;
        if (cluster_id != _id) {
          // Voxel is in another cluster.
          return false;
        }
      }
    }
  }

  return true;
}
