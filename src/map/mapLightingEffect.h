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
  static CPT(RenderEffect) make();

  const RenderState *get_current_lighting_state() const;

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

private:
  // Cached data to determine if we need to recompute the node's lighting.
  CPT(TransformState) _last_transform;
  const MapData *_last_map_data;

  // This is the actual lighting state.
  PT(Texture) _cube_map;
  CPT(RenderState) _lighting_state;
  PTA_LVecBase3 _probe_color;
  const MapAmbientProbe *_probe;
  CPT(RenderAttrib) _light_attrib;
  CPT(RenderAttrib) _modified_light_attrib;
};

#include "mapLightingEffect.I"

#endif // MAPLIGHTINGEFFECT_H
