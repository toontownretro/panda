/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapBuildOptions.h
 * @author brian
 * @date 2021-07-05
 */

#ifndef MAPBUILDOPTIONS_H
#define MAPBUILDOPTIONS_H

#include "pandabase.h"
#include "filename.h"
#include "luse.h"

/**
 * Contains all of the configuration options for building a map.
 */
class EXPCL_PANDA_MAPBUILDER MapBuildOptions {
PUBLISHED:
  INLINE MapBuildOptions();

  INLINE void set_input_filename(const Filename &filename);
  INLINE Filename get_input_filename() const;

  INLINE void set_output_filename(const Filename &filename);
  INLINE Filename get_output_filename() const;

  INLINE void set_csg(bool flag);
  INLINE bool get_csg() const;

  INLINE void set_partition(bool flag);
  INLINE bool get_partition() const;

  INLINE void set_vis(bool flag);
  INLINE bool get_vis() const;

  INLINE void set_light(bool flag);
  INLINE bool get_light() const;

  INLINE void set_num_threads(int count);
  INLINE int get_num_threads() const;

  INLINE void set_vis_voxel_size(const LVecBase3 &size);
  INLINE const LVecBase3 &get_vis_voxel_size() const;

  INLINE void set_vis_tile_size(const LVecBase3i &size);
  INLINE const LVecBase3i &get_vis_tile_size() const;

  INLINE void set_vis_max_cell_size(const LVecBase3 &size);
  INLINE const LVecBase3 &get_vis_max_cell_size() const;

  INLINE void set_vis_show_solid_voxels(bool flag);
  INLINE bool get_vis_show_solid_voxels() const;

  INLINE void set_vis_show_areas(bool flag);
  INLINE bool get_vis_show_areas() const;

  INLINE void set_vis_show_portals(bool flag);
  INLINE bool get_vis_show_portals() const;

  INLINE void set_mesh_group_size(PN_stdfloat size);
  INLINE PN_stdfloat get_mesh_group_size() const;

  INLINE void set_steam_audio(bool flag);
  INLINE bool get_steam_audio() const;

  INLINE void set_steam_audio_reflections(bool flag);
  INLINE bool get_steam_audio_reflections() const;

  INLINE void set_steam_audio_pathing(bool flag);
  INLINE bool get_steam_audio_pathing() const;

public:
  Filename _input_filename;
  Filename _output_filename;
  bool _do_csg; // Perform CSG on intersecting solids.
  bool _do_partition; // Compute spatial partition structure.
  bool _do_vis; // Compute potentially visible set.
  bool _do_light; // Compute lighting information.
  int _num_threads;

  bool _do_steam_audio;
  bool _do_steam_audio_reflections;
  bool _do_steam_audio_pathing;

  bool _vis_show_solid_voxels;
  bool _vis_show_areas;
  bool _vis_show_portals;

  LVecBase3 _vis_voxel_size;
  LVecBase3i _vis_tile_size;
  LVecBase3 _vis_max_cell_size;

  PN_stdfloat _mesh_group_size;
};

#include "mapBuildOptions.I"

#endif // MAPBUILDOPTIONS_H
