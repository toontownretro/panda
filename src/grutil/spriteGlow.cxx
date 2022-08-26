/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteGlow.cxx
 * @author brian
 * @date 2022-06-17
 */

#include "spriteGlow.h"
#include "geomDrawCallbackData.h"
#include "sceneSetup.h"
#include "graphicsStateGuardian.h"
#include "renderState.h"
#include "colorScaleAttrib.h"
#include "callbackObject.h"
#include "cullHandler.h"
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "internalName.h"
#include "shaderAttrib.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "colorWriteAttrib.h"
#include "colorBlendAttrib.h"
#include "transparencyAttrib.h"
#include "antialiasAttrib.h"
#include "cullFaceAttrib.h"
#include "geomTristrips.h"
#include "cullBinAttrib.h"

IMPLEMENT_CLASS(SpriteGlow);

class SpriteGlowDrawCallback : public CallbackObject {
public:
  SpriteGlowDrawCallback(SpriteGlow *glow);

  virtual void do_callback(CallbackData *cbdata) override;

private:
  SpriteGlow *_glow;
};

/**
 *
 */
SpriteGlowDrawCallback::
SpriteGlowDrawCallback(SpriteGlow *glow) :
  _glow(glow)
{
}

/**
 *
 */
void SpriteGlowDrawCallback::
do_callback(CallbackData *cbdata) {
  _glow->draw_callback((GeomDrawCallbackData *)cbdata);
}

/**
 *
 */
SpriteGlow::
SpriteGlow(const std::string &name, PN_stdfloat radius) :
  PandaNode(name),
  _radius(radius)
{
  set_renderable();
  set_cull_callback();
  init_geoms();
}

/**
 *
 */
bool SpriteGlow::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  SceneSetup *scene = trav->get_scene();
  Camera *cam = scene->get_camera_node();

  CamQueryData &query_data = _contexts[cam];

  std::cout << query_data.ctx << ", " << query_data.count_ctx << ", "
    << query_data.num_passed << ", " << query_data.num_possible << "\n";

  if (query_data.num_passed > 0) {
    std::cout << query_data.num_passed << " / " << query_data.num_possible << "\n";
    float fraction = (float)query_data.num_passed / (float)query_data.num_possible;
    if (fraction < 1.0f) {
      // Blend fraction.
      data._state = data._state->compose(
        RenderState::make(
          ColorScaleAttrib::make(LVecBase4(fraction, fraction, fraction, 1.0f))));
    }

    // Keep traversing.
    return true;

  } else {
    // No pixels passed query, so we are not visible at all.
    return true;
  }
}

/**
 *
 */
void SpriteGlow::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  std::cout << "Add for draw\n";
  CullableObject *obj = new CullableObject(nullptr, RenderState::make_empty(), data.get_internal_transform(trav), trav->get_current_thread());
  obj->set_draw_callback(new SpriteGlowDrawCallback(this));
  trav->get_cull_handler()->record_object(obj, trav);
}

/**
 *
 */
void SpriteGlow::
draw_callback(GeomDrawCallbackData *cbdata) {
  std::cout << "Draw callback\n";
  GraphicsStateGuardian *gsg = (GraphicsStateGuardian *)cbdata->get_gsg();
  CullableObject *obj = cbdata->get_object();
  SceneSetup *scene = gsg->get_scene();
  Camera *cam = scene->get_camera_node();

  CamQueryData &query_data = _contexts[cam];

  if (query_data.ctx == nullptr || query_data.count_ctx == nullptr) {
    // No active query context, issue a new one.
    issue_query(query_data, gsg, obj->_internal_transform);

  } else {
    // There is an active query, check if the answer is ready.
    if (query_data.ctx->is_answer_ready() && query_data.count_ctx->is_answer_ready()) {
      query_data.num_passed = query_data.ctx->get_num_fragments();
      query_data.num_possible = query_data.count_ctx->get_num_fragments();

      // Now issue a new query.
      issue_query(query_data, gsg, obj->_internal_transform);
    }
  }

  cbdata->set_lost_state(false);
}

/**
 *
 */
void SpriteGlow::
issue_query(CamQueryData &query_data, GraphicsStateGuardian *gsg, const TransformState *transform) {
  gsg->ensure_generated_shader(_query_count_state);
  gsg->ensure_generated_shader(_query_state);

  // First do a query without depth-testing to see how many fragments are
  // possible.
  gsg->begin_occlusion_query();

  // Render the query geometry with the count query state.
  gsg->set_state_and_transform(_query_count_state, transform);
  gsg->draw_geom(_query_point, _query_point->get_vertex_data(), 1,
                 _query_prim, true, Thread::get_current_thread());


  query_data.count_ctx = gsg->end_occlusion_query();

  gsg->begin_occlusion_query();

  // Now render the actual query.
  gsg->set_state_and_transform(_query_state, transform);
  gsg->draw_geom(_query_point, _query_point->get_vertex_data(), 1,
                 _query_prim, true, Thread::get_current_thread());

  query_data.ctx = gsg->end_occlusion_query();
}

/**
 *
 */
void SpriteGlow::
init_geoms() {
  PT(GeomVertexData) vdata = new GeomVertexData("glow-query", GeomVertexFormat::get_v3(),
                                                GeomEnums::UH_static);

  GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
  vwriter.add_data3f(-_radius, 0, -_radius); // ll
  vwriter.add_data3f(_radius, 0, -_radius); // lr
  vwriter.add_data3f(_radius, 0, _radius); // ur
  vwriter.add_data3f(-_radius, 0, _radius); // ul

  PT(GeomTristrips) prim = new GeomTristrips(GeomEnums::UH_static);
  prim->add_next_vertices(4);
  prim->close_primitive();

  _query_prim = prim;

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(prim);
  _query_point = geom;

  CPT(RenderState) state = RenderState::make_empty();
  state = state->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_none));
  state = state->set_attrib(AntialiasAttrib::make(AntialiasAttrib::M_none));
  state = state->set_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
  state = state->set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off));
  state = state->set_attrib(ColorBlendAttrib::make(ColorBlendAttrib::M_none));
  state = state->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_less));
  state = state->set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
  state = state->set_attrib(CullBinAttrib::make("unsorted", 10));

  _query_state = state;

  // Don't do depth-test when counting possible number of fragments.
  state = state->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
  _query_count_state = state;
}
