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
#include "renderModeAttrib.h"
#include "cmath.h"
#include "lightMutexHolder.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector sprite_glow_dc_pcollector("Draw:SpriteGlowCallback");

IMPLEMENT_CLASS(SpriteGlow);

class SpriteGlowDrawCallback : public CallbackObject {
public:
  SpriteGlowDrawCallback(SpriteGlow *glow);

  virtual void do_callback(CallbackData *cbdata) override;

  CPT(TransformState) _count_query_transform;

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
  _glow->draw_callback((GeomDrawCallbackData *)cbdata, _count_query_transform);
}

/**
 *
 */
SpriteGlow::
SpriteGlow(const std::string &name, PN_stdfloat radius, bool perspective) :
  PandaNode(name),
  _radius(radius),
  _perspective(perspective),
  _contexts_lock("sprite-glow-contexts-lock"),
  _query_pixel_size((int)roundf(powf(_radius, 2.0f)))
{
  set_renderable();
  init_geoms();
}

/**
 *
 */
void SpriteGlow::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  // Add it with the query state so it's drawn in the unsorted bin, in front
  // of everything else.  We need the rest of the scene to be drawn so we
  // can correctly depth test the query geometry.
  //std::cout << "sg add for darw\n";

  CPT(TransformState) transform = data.get_internal_transform(trav);
  SpriteGlowDrawCallback *clbk = new SpriteGlowDrawCallback(this);
  if (_perspective) {
    clbk->_count_query_transform = TransformState::make_pos(LVecBase3(0, transform->get_pos().length(), 0));
    trav->get_gsg()->ensure_generated_shader(_query_count_state);
  }
  CullableObject obj(nullptr, _query_state, std::move(transform), trav->get_current_thread());
  obj.set_draw_callback(clbk);
  trav->get_cull_handler()->record_object(&obj, trav);
}

/**
 *
 */
void SpriteGlow::
draw_callback(GeomDrawCallbackData *cbdata, const TransformState *count_transform) {
  //std::cout << "Dc\n";
  PStatTimer timer(sprite_glow_dc_pcollector);

  GraphicsStateGuardian *gsg = (GraphicsStateGuardian *)cbdata->get_gsg();
  CullableObject *obj = cbdata->get_object();
  SceneSetup *scene = gsg->get_scene();
  Camera *cam = scene->get_camera_node();

  CamQueryData *query_data = find_or_create_query_data(cam);

  if (query_data->valid) {
    if (query_data->ctx == nullptr || (_perspective && query_data->count_ctx == nullptr)) {
      // No active query context, issue a new one.
      issue_query(*query_data, gsg, obj->_internal_transform, count_transform);

    } else {
      // There is an active query, check if the answer is ready.
      int ctx_passed = query_data->ctx->get_num_fragments(false);
      int count_ctx_passed = -1;
      if (_perspective) {
        count_ctx_passed = query_data->count_ctx->get_num_fragments(false);
      }
      if ((ctx_passed != -1) && (!_perspective || (count_ctx_passed != -1))) {
        AtomicAdjust::set(query_data->num_passed, ctx_passed);
        if (_perspective) {
          AtomicAdjust::set(query_data->num_possible, count_ctx_passed);
        }

        //std::cout  << query_data->num_passed << " / " << query_data->num_possible << "\n";

        // Now issue a new query.
        issue_query(*query_data, gsg, obj->_internal_transform, count_transform);
      }
    }
  }

  cbdata->set_lost_state(false);
}

/**
 *
 */
void SpriteGlow::
issue_query(CamQueryData &query_data, GraphicsStateGuardian *gsg, const TransformState *transform,
            const TransformState *count_transform) {
  const GeomVertexData *vdata = _query_point->get_vertex_data_noref();
  Thread *current_thread = Thread::get_current_thread();

  //std::cout << "isue qyery\n";

  if (query_data.ctx == nullptr) {
    query_data.ctx = gsg->create_occlusion_query();
  }
  if (query_data.ctx != nullptr) {
    //gsg->ensure_generated_shader(_query_state);
    //gsg->set_state_and_transform(_query_state, transform);
    // The query state is already set since it's the state we set on the CullableObject with
    // the draw callback.
    gsg->begin_occlusion_query(query_data.ctx);
    // Now render the actual query.
    gsg->draw_geom(_query_point, vdata, 1,
                  _query_prim, true, current_thread);
    gsg->end_occlusion_query();
  } else {
    query_data.valid = false;
  }

  if (_perspective) {
    if (query_data.count_ctx == nullptr) {
      query_data.count_ctx = gsg->create_occlusion_query();
    }
    if (query_data.count_ctx != nullptr) {
      // Render the "count" query directly in front of the camera, offset forward
      // the same distance from the camera as the actual query.  This will give us
      // a rough estimate of the number of "possible" fragments for the query.

      // Render the query geometry with the count query state.
      //gsg->ensure_generated_shader(_query_count_state);
      gsg->set_state_and_transform(_query_count_state, count_transform);
      // First do a query without depth-testing to see how many fragments are
      // possible.
      gsg->begin_occlusion_query(query_data.count_ctx);
      gsg->draw_geom(_query_point, vdata, 1,
                    _query_prim, true, current_thread);
      gsg->end_occlusion_query();
    } else {
      query_data.valid = false;
    }
  }
}

/**
 *
 */
void SpriteGlow::
init_geoms() {
  PT(GeomVertexData) vdata = new GeomVertexData("glow-query", GeomVertexFormat::get_v3(),
                                                GeomEnums::UH_static);

  GeomVertexWriter vwriter(vdata, InternalName::get_vertex());

  if (_perspective) {
    // In perspective mode, the query geometry is a quad with a world-space
    // radius.  In this mode, we issue two queries, one to count the possible
    // number of fragments for the query, by rendering it directly in front of
    // the camera (at the same distance from the camera as the regular query),
    // and the actual query with depth-testing enabled to count the number
    // of visible fragments from the actual query location.
    vwriter.add_data3f(-_radius, 0, -_radius); // ll
    vwriter.add_data3f(_radius, 0, -_radius); // lr
    vwriter.add_data3f(_radius, 0, _radius); // ur
    vwriter.add_data3f(-_radius, 0, _radius); // ul

    PT(GeomTriangles) prim = new GeomTriangles(GeomEnums::UH_static);
    prim->add_vertices(0, 1, 2);
    prim->close_primitive();
    prim->add_vertices(2, 3, 0);
    prim->close_primitive();
    _query_prim = prim;

  } else {
    // In non-perspective mode, the query geometry is a point with a
    // screen-space pixel thickness.  Since it's a constant screen-space
    // point, we can estimate the number of possible fragments without
    // needing another "counting" query as in the perspective mode above.
    vwriter.add_data3f(0, 0, 0);

    PT(GeomPoints) prim = new GeomPoints(GeomEnums::UH_static);
    prim->add_vertex(0);
    prim->close_primitive();
    _query_prim = prim;
  }


  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(_query_prim);
  _query_point = geom;

  CPT(RenderState) state = RenderState::make_empty();
  state = state->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_none));
  state = state->set_attrib(AntialiasAttrib::make(AntialiasAttrib::M_none));
  state = state->set_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
  state = state->set_attrib(ColorWriteAttrib::make(ColorWriteAttrib::C_off));
  state = state->set_attrib(ColorBlendAttrib::make(ColorBlendAttrib::M_none));
  state = state->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_less));
  state = state->set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_unchanged));
  state = state->set_attrib(CullBinAttrib::make("unsorted", 10));
  if (!_perspective) {
    // In non-perspective mode, the query is a single point with a screen-space point size.
    state = state->set_attrib(RenderModeAttrib::make(RenderModeAttrib::M_point, _radius));
  }

  _query_state = state;

  // Don't do depth-test when counting possible number of fragments.
  state = state->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
  _query_count_state = state;
}

/**
 * Returns a 0-1 fraction representing the number of fragments that passed the
 * query compared to the number of possible fragments of the query geometry.
 */
PN_stdfloat SpriteGlow::
get_fraction_visible(Camera *cam) const {
  CamQueryData *data = get_query_data(cam);
  if (data == nullptr) {
    return 0.0f;
  }

  int num_possible;
  if (_perspective) {
    num_possible = AtomicAdjust::get(data->num_possible);
  } else {
    num_possible = _query_pixel_size;
  }
  if (num_possible > 0) {
    int num_passed = AtomicAdjust::get(data->num_passed);
    return std::min(1.0f, (float)num_passed / (float)num_possible);
  }

  return 0.0f;
}

/**
 *
 */
SpriteGlow::CamQueryData *SpriteGlow::
find_or_create_query_data(Camera *cam) {
  LightMutexHolder holder(_contexts_lock);
  PT(CamQueryData) &data = _contexts[cam];
  if (data == nullptr) {
    data = new CamQueryData;
    if (!_perspective) {
      data->num_possible = _query_pixel_size;
    }
  }
  return data;
}

/**
 *
 */
SpriteGlow::CamQueryData *SpriteGlow::
get_query_data(Camera *cam) const {
  LightMutexHolder holder(_contexts_lock);
  CameraContexts::const_iterator it = _contexts.find(cam);
  if (it != _contexts.end()) {
    return (*it).second;
  }
  return nullptr;
}
