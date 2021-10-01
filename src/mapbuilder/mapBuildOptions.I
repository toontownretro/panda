/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapBuildOptions.I
 * @author brian
 * @date 2021-07-05
 */

/**
 *
 */
INLINE MapBuildOptions::
MapBuildOptions() {
  _do_csg = true;
  _do_partition = true;
  _do_vis = true;
  _do_light = true;
  _num_threads = 0;
  _vis_show_solid_voxels = false;
  _vis_show_areas = false;
  _vis_show_portals = false;
  _vis_voxel_size.set(16, 16, 16);
  _vis_max_cell_size.set(1e+9, 1e+9, 1e+9);
  _vis_tile_size.set(128, 128, 128);
  _mesh_group_size = 256.0f;
}

/**
 * Sets the filename of the map file to be built.
 */
INLINE void MapBuildOptions::
set_input_filename(const Filename &filename) {
  _input_filename = filename;
}

/**
 * Returns the filename of the map file to be built.
 */
INLINE Filename MapBuildOptions::
get_input_filename() const {
  return _input_filename;
}

/**
 * Sets the filename to write the built map to.
 */
INLINE void MapBuildOptions::
set_output_filename(const Filename &filename) {
  _output_filename = filename;
}

/**
 * Returns the filename to write the built map to.
 */
INLINE Filename MapBuildOptions::
get_output_filename() const {
  return _output_filename;
}

/**
 * Sets whether or not to subtract intersecting CSG solids.
 */
INLINE void MapBuildOptions::
set_csg(bool flag) {
  _do_csg = flag;
}

/**
 * Returns true if intersecting CSG solids should be subtracted.
 */
INLINE bool MapBuildOptions::
get_csg() const {
  return _do_csg;
}

/**
 * Sets whether or not to build the spatial partition structure.
 */
INLINE void MapBuildOptions::
set_partition(bool flag) {
  _do_partition = flag;
}

/**
 * Returns true if the spatial partition structure should be built.
 */
INLINE bool MapBuildOptions::
get_partition() const {
  return _do_partition;
}

/**
 * Sets whether or not to build the potentially visible set (PVS).  Requires
 * the partition to be enabled.
 */
INLINE void MapBuildOptions::
set_vis(bool flag) {
  _do_vis = flag;
}

/**
 * Returns true if the potentially visible set (PVS) should be built.
 */
INLINE bool MapBuildOptions::
get_vis() const {
  return _do_vis;
}

/**
 * Sets whether or not to build lighting information.
 */
INLINE void MapBuildOptions::
set_light(bool flag) {
  _do_light = flag;
}

/**
 * Returns true if lighting information should be built.
 */
INLINE bool MapBuildOptions::
get_light() const {
  return _do_light;
}

/**
 * Sets the number of threads that should be used to distribute work.  If this
 * is 0 or less, the builder will use the number of threads available to the
 * system.
 */
INLINE void MapBuildOptions::
set_num_threads(int count) {
  _num_threads = count;
}

/**
 * Returns the number of thread that should be used to distributed work.  If
 * this is 0 or less, the builder will use the number of threads available to
 * the system.
 */
INLINE int MapBuildOptions::
get_num_threads() const {
  return _num_threads;
}

/**
 * Sets the size of each voxel in the vis pass.
 */
INLINE void MapBuildOptions::
set_vis_voxel_size(const LVecBase3 &size) {
  _vis_voxel_size = size;
}

/**
 * Returns the size of each voxel in the vis pass.
 */
INLINE const LVecBase3 &MapBuildOptions::
get_vis_voxel_size() const {
  return _vis_voxel_size;
}

/**
 * Sets the maximum cell size in the vis pass.
 */
INLINE void MapBuildOptions::
set_vis_max_cell_size(const LVecBase3 &size) {
  _vis_max_cell_size = size;
}

/**
 * Returns the maximum cell size in the vis pass.
 */
INLINE const LVecBase3 &MapBuildOptions::
get_vis_max_cell_size() const {
  return _vis_max_cell_size;
}

/**
 * Sets the size of each tile in the vis pass.
 */
INLINE void MapBuildOptions::
set_vis_tile_size(const LVecBase3i &size) {
  _vis_tile_size = size;
}

/**
 * Returns the size of each tile in the vis pass.
 */
INLINE const LVecBase3i &MapBuildOptions::
get_vis_tile_size() const {
  return _vis_tile_size;
}

/**
 * Enables or disables debug visualization of solid voxels during the vis
 * pass.
 */
INLINE void MapBuildOptions::
set_vis_show_solid_voxels(bool flag) {
  _vis_show_solid_voxels = flag;
}

/**
 * Returns true if solid voxels should be visualized.
 */
INLINE bool MapBuildOptions::
get_vis_show_solid_voxels() const {
  return _vis_show_solid_voxels;
}

/**
 * Enables or disables debug visualization of visibility areas.
 */
INLINE void MapBuildOptions::
set_vis_show_areas(bool flag) {
  _vis_show_areas = flag;
}

/**
 * Returns true if visibility areas should be visualized.
 */
INLINE bool MapBuildOptions::
get_vis_show_areas() const {
  return _vis_show_areas;
}

/**
 * Enables or disables debug visualization of portals.
 */
INLINE void MapBuildOptions::
set_vis_show_portals(bool flag) {
  _vis_show_portals = flag;
}

/**
 * Returns true if portals should be visualized.
 */
INLINE bool MapBuildOptions::
get_vis_show_portals() const {
  return _vis_show_portals;
}

/**
 * Sets the cubic size in world units of mesh groups.  The larger the size,
 * the more meshes and polygons will be flattened together, but culling will
 * be less effective.  Each map may have a different ideal mesh group size.
 */
INLINE void MapBuildOptions::
set_mesh_group_size(PN_stdfloat size) {
  _mesh_group_size = size;
}

/**
 * Returns the cubic size in world units of mesh groups.
 *
 * See set_mesh_group_size().
 */
INLINE PN_stdfloat MapBuildOptions::
get_mesh_group_size() const {
  return _mesh_group_size;
}
