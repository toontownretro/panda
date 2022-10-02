/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spotlightBeam.cxx
 * @author brian
 * @date 2022-09-28
 */

#include "spotlightBeam.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cmath.h"
#include "colorScaleAttrib.h"
#include "mathutil_misc.h"
#include "boundingSphere.h"
#include "billboardEffect.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector sb_cull_pcollector("Cull:SpotlightBeamCullCallback");

IMPLEMENT_CLASS(SpotlightBeam);

/**
 *
 */
SpotlightBeam::
SpotlightBeam(const std::string &name) :
  PandaNode(name),
  _beam_color(1.0f, 1.0f, 1.0f, 1.0f),
  _halo_color(1.0f),
  _beam_length(128.0f),
  _beam_width(32.0f),
  _halo_size(32.0f),
  _halo_query(new SpriteGlow("spotlight-beam-halo", 10.0f, true))
{
  _halo_query->set_effect(BillboardEffect::make_point_eye());
  set_cull_callback();
  set_renderable();
  set_bounds(new BoundingSphere(LPoint3(0.0), 10.0f));
}

/**
 *
 */
float
closest_point_to_line_t(const LPoint3 &p, const LPoint3 &a, const LPoint3 &b, LVector3 &dir) {
  dir = b - a;

  float div = dir.dot(dir);
  if (div < 0.00001f) {
    return 0.0f;
  } else {
    return (dir.dot(p) - dir.dot(a)) / div;
  }
}

/**
 *
 */
void
closest_point_on_line(const LPoint3 &p, const LPoint3 &a, const LPoint3 &b, LPoint3 &closest, float *out_t) {
  LVector3 dir;
  float t = closest_point_to_line_t(p, a, b, dir);
  if (out_t != nullptr) {
    *out_t = t;
  }
  closest = a + dir * t;
}

/**
 *
 */
bool SpotlightBeam::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  PStatTimer timer(sb_cull_pcollector);

  if (data._node_reader.get_num_children() == 0) {
    return false;
  }

  const TransformState *net_transform = data._net_transform;
  LPoint3 beam_pos = net_transform->get_pos();
  LQuaternion quat = net_transform->get_quat();
  LVector3 beam_dir = quat.get_forward();

  LPoint3 view_pos = trav->get_camera_transform()->get_pos();

  LVector3 local_dir = view_pos - beam_pos;
  local_dir.normalize();

  float fade;
  PN_stdfloat dot = beam_dir.dot(local_dir);
  if (dot < 0.0f) {
    fade = 0.0f;
  } else {
    fade = dot * 2.0f;
  }

  float dist_to_line;
  LPoint3 closest_point;
  // Find out how close we are to the "line" of the spotlight.
  closest_point_on_line(view_pos, beam_pos, beam_pos + beam_dir * 2, closest_point, nullptr);
  dist_to_line = (view_pos - closest_point).length();

  float dot_scale = 1.0f;
  float dist_threshold = _beam_width * 4.0f;
  if (dist_to_line < dist_threshold) {
    dot_scale = remap_val_clamped(dist_to_line, dist_threshold, _beam_width, 1.0f, 0.0f);
    dot_scale = std::max(0.0f, std::min(1.0f, dot_scale));
  }

  if (!IS_NEARLY_ZERO(dot_scale)) {
    LColor scale_color = _beam_color * dot_scale;
    CPT(RenderState) beam_state =
      data._state->compose(RenderState::make(ColorScaleAttrib::make(scale_color)));
    trav->traverse_down(data, data._node_reader.get_child_connection(0), beam_state);
  }

  if (data._node_reader.get_num_children() < 2) {
    return false;
  }

  float halo_vis = _halo_query->get_fraction_visible(trav->get_scene()->get_camera_node());
  if (!IS_NEARLY_ZERO(fade) && !IS_NEARLY_ZERO(halo_vis)) {
    float halo_scale = remap_val_clamped(dist_to_line, dist_threshold, _beam_width * 0.5f, 1.0f, 2.0f);
    halo_scale = std::max(1.0f, std::min(2.0f, halo_scale));
    halo_scale *= _halo_size;

    float color_fade = fade * fade;
    color_fade = std::max(0.0f, std::min(1.0f, color_fade));

    LColor halo_color = _halo_color * color_fade * halo_vis;
    CPT(RenderState) halo_rs =
      data._state->compose(RenderState::make(ColorScaleAttrib::make(halo_color)));
    CPT(TransformState) halo_ts =
      data._net_transform->set_scale(data._net_transform->get_scale() * halo_scale);
    trav->traverse_down(data, data._node_reader.get_child(1), halo_ts, halo_rs);
  }

  trav->traverse_down(data, _halo_query);

  return false;
}

/**
 *
 */
void SpotlightBeam::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  //std::cout << "Add for draw\n";
  //_halo_query->add_for_draw(trav, data);
}
