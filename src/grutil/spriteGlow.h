/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteGlow.h
 * @author brian
 * @date 2022-06-17
 */

#ifndef SPRITEGLOW_H
#define SPRITEGLOW_H

#include "pandabase.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "occlusionQueryContext.h"
#include "camera.h"
#include "geom.h"
#include "renderState.h"
#include "geomPoints.h"

class CullTraverser;
class CullTraverserData;
class GeomDrawCallbackData;
class GraphicsStateGuardian;

/**
 *
 */
class EXPCL_PANDA_GRUTIL SpriteGlow : public PandaNode {
  DECLARE_CLASS(SpriteGlow, PandaNode);

PUBLISHED:
  SpriteGlow(const std::string &name, PN_stdfloat radius);

public:
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data) override;

  void draw_callback(GeomDrawCallbackData *cbdata);

  void init_geoms();

private:
  struct CamQueryData {
    PT(OcclusionQueryContext) ctx = nullptr;
    PT(OcclusionQueryContext) count_ctx = nullptr;
    int num_passed = 0;
    int num_possible = 0;
  };

  typedef pmap<WPT(Camera), CamQueryData> CameraContexts;
  CameraContexts _contexts;

  CPT(Geom) _query_point;
  CPT(RenderState) _query_state;
  CPT(RenderState) _query_count_state;
  CPT(GeomPrimitive) _query_prim;
  int _query_pixel_size;

  PN_stdfloat _radius;

private:
  void issue_query(CamQueryData &data, GraphicsStateGuardian *gsg, const TransformState *transform);
};

#include "spriteGlow.I"

#endif // SPRITEGLOW_H
