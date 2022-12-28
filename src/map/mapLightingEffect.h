/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapLightingEffect.h
 * @author brian
 * @date 2021-12-06
 */

#ifndef MAPLIGHTINGEFFECT_H
#define MAPLIGHTINGEFFECT_H

#include "pandabase.h"
#include "renderEffect.h"
#include "transformState.h"
#include "boundingVolume.h"
#include "pta_LVecBase3.h"
#include "renderAttrib.h"
#include "texture.h"
#include "nodePath.h"

class MapAmbientProbe;
class CullTraverser;
class CullTraverserData;
class MapData;

/**
 * This is a special RenderEffect that applies lighting state to nodes
 * from the lighting information in the map.
 */
class EXPCL_PANDA_MAP MapLightingEffect : public RenderEffect {
  DECLARE_CLASS(MapLightingEffect, RenderEffect);

PUBLISHED:
  enum Flags {
    F_probe = 1,
    F_cube_map = 2,
    F_static_lights = 4,
    F_dynamic_lights = 8,
    F_force_sun = 16,
    F_no_sun = 32,

    F_default_dynamic = F_probe|F_cube_map|F_static_lights|F_dynamic_lights,
    F_default_baked = F_cube_map|F_dynamic_lights|F_force_sun,
    F_default_baked_3d_sky = F_cube_map|F_no_sun,
  };

  static CPT(RenderEffect) make(BitMask32 camera_mask, bool use_position = true, unsigned int flags = F_default_dynamic, int max_lights = 4);
  static CPT(RenderEffect) make(BitMask32 camera_mask, const LPoint3 &lighting_origin, unsigned int flags = F_default_dynamic, int max_lights = 4);

  const RenderState *get_current_lighting_state() const;

  void compute_lighting(const TransformState *net_transform, MapData *map_data,
                        const GeometricBoundingVolume *node_bounds,
                        const TransformState *parent_net_transform) const;

  void do_compute_lighting(const TransformState *net_transform, MapData *map_data,
                           const GeometricBoundingVolume *node_bounds,
                           const TransformState *parent_net_transform);

  static void mark_stale();
  static void set_dynamic_light_root(NodePath np);
  static void clear_dynamic_light_root();

  ~MapLightingEffect();

public:
  virtual bool has_cull_callback() const override;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const override;

protected:
  MapLightingEffect();


  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  void do_cull_callback(CullTraverser *trav, CullTraverserData &data,
                        CPT(TransformState) &node_transform,
                        CPT(RenderState) &node_state);

  void add_to_linked_list();
  void remove_from_linked_list();

private:
  // Cached data to determine if we need to recompute the node's lighting.
  LPoint3 _last_pos;
  const MapData *_last_map_data;

  // True if we should use the node's position as the lighting origin
  // rather than the bounding volume center if an explicit lighting
  // origin was not specified.
  bool _use_position;
  bool _has_lighting_origin;
  LPoint3 _lighting_origin;

  unsigned int _flags;

  int _max_lights;

  // This is the actual lighting state.
  PT(Texture) _cube_map;
  CPT(RenderState) _lighting_state;
  PTA_LVecBase3 _probe_color;
  const MapAmbientProbe *_probe;

  BitMask32 _camera_mask;

  const MapLightingEffect *_next;

  UpdateSeq _last_update;

  static const MapLightingEffect *_list;
  static UpdateSeq _next_update;
  static NodePath _dynamic_light_root;
};

#include "mapLightingEffect.I"

#endif // MAPLIGHTINGEFFECT_H
