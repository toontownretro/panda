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
#include "lightMutex.h"
#include "referenceCount.h"
#include "atomicAdjust.h"

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
  SpriteGlow(const std::string &name, PN_stdfloat radius, bool perspective);

  PN_stdfloat get_fraction_visible(Camera *cam) const;

public:
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data) override;

  void draw_callback(GeomDrawCallbackData *cbdata, const TransformState *count_transform);

  void init_geoms();

private:
  class CamQueryData : public ReferenceCount {
  public:
    CamQueryData() = default;

    CPT(TransformState) count_query_transform;

    PT(OcclusionQueryContext) ctx = nullptr;
    PT(OcclusionQueryContext) count_ctx = nullptr;
    AtomicAdjust::Integer num_passed = 0;
    AtomicAdjust::Integer num_possible = 0;

    bool valid = true;
  };
  CamQueryData *find_or_create_query_data(Camera *cam);
  CamQueryData *get_query_data(Camera *cam) const;

  typedef pmap<WPT(Camera), PT(CamQueryData)> CameraContexts;
  CameraContexts _contexts;
  LightMutex _contexts_lock;

  CPT(Geom) _query_point;
  CPT(RenderState) _query_state;
  CPT(RenderState) _query_count_state;
  CPT(GeomPrimitive) _query_prim;
  int _query_pixel_size;

  PN_stdfloat _radius;

  bool _perspective;

private:
  void issue_query(CamQueryData &data, GraphicsStateGuardian *gsg, const TransformState *transform, const TransformState *count_transform);
};

#include "spriteGlow.I"

#endif // SPRITEGLOW_H
