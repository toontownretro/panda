/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneVisibility.I
 * @author brian
 * @date 2021-11-17
 */

/**
 * Returns true if the given point in space is within any sectors of the
 * given PVS, false otherwise.
 */
INLINE bool SceneVisibility::
is_point_in_pvs(const LPoint3 &point, const BitArray &pvs) const {
  int point_sector = _sector_tree.get_leaf_value_from_point(point);
  if (point_sector < 0) {
    return false;
  }
  return pvs.get_bit(point_sector);
}

/**
 * Returns the ID of the sector that contains the given point in space, or -1
 * if the point is outside the interior space of the world.
 */
INLINE int SceneVisibility::
get_point_sector(const LPoint3 &point) const {
  return _sector_tree.get_leaf_value_from_point(point);
}

/**
 * Specifies the potentially visible set for a sector.  The sector the
 * PVS corresponds to is the number of PVS's added so far.
 */
INLINE void SceneVisibility::
add_sector_pvs(const BitArray &pvs) {
  _sector_pvs.push_back(pvs);
}

/**
 * Returns the number of visibility sectors.  Each sector has a PVS associated
 * with it.
 */
INLINE int SceneVisibility::
get_num_sectors() const {
  return (int)_sector_pvs.size();
}

/**
 * Returns a BitArray of sectors that are potentially visible from the given
 * sector.
 */
INLINE const BitArray *SceneVisibility::
get_sector_pvs(int sector) const {
  nassertr(sector >= 0 && sector < (int)_sector_pvs.size(), nullptr);
  return &_sector_pvs[sector];
}

/**
 * Specifies the spatial search structure for locating sectors from points and
 * volumes.
 */
INLINE void SceneVisibility::
set_sector_tree(const KDTree &tree) {
  _sector_tree = tree;
}

/**
 * Returns the spatial search structure for locating sectors from points and
 * volumes.
 */
INLINE const KDTree *SceneVisibility::
get_sector_tree() const {
  return &_sector_tree;
}

/**
 * Clears the per-node visibility cache.
 */
INLINE void SceneVisibility::
clear_node_vis_cache() {
  _node_vis_cache.clear();
}
