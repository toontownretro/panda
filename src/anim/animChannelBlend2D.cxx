/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBlend2D.cxx
 * @author brian
 * @date 2021-08-04
 */

#include "animChannelBlend2D.h"
#include "triangulatorDelaunay.h"
#include "poseParameter.h"
#include "character.h"

IMPLEMENT_CLASS(AnimChannelBlend2D);

static const PN_stdfloat equal_epsilon = 0.001f;

/**
 *
 */
AnimChannelBlend2D::
AnimChannelBlend2D(const std::string &name) :
  AnimChannel(name),
  _has_triangles(false),
  _blend_x(-1),
  _blend_y(-1),
  _active_tri(nullptr)
{
}

/**
 * Builds a set of triangles out of all input points.
 */
void AnimChannelBlend2D::
build_triangles() {
  _triangles.clear();

  TriangulatorDelaunay triangles;
  for (size_t i = 0; i < _channels.size(); i++) {
    triangles.add_point(_channels[i]._point);
  }
  triangles.triangulate();

  for (size_t i = 0; i < triangles.get_num_triangles(); i++) {
    const TriangulatorDelaunay::Triangle &dtri = triangles.get_triangle(i);
    Triangle tri;
    tri.a = dtri._a;
    tri.b = dtri._b;
    tri.c = dtri._c;
    _triangles.push_back(tri);
  }

  _has_triangles = true;
}

/**
 * Builds triangles and computes weights if something changed.
 */
void AnimChannelBlend2D::
compute_weights_if_necessary(Character *character) {
  LPoint2 input(0);
  if (_blend_x != -1) {
    const PoseParameter &x = character->get_pose_parameter(_blend_x);
    input[0] = x.get_norm_value();
  }
  if (_blend_y != -1) {
    const PoseParameter &y = character->get_pose_parameter(_blend_y);
    input[1] = y.get_norm_value();
  }
  if ((input != _input_coord) || !_has_triangles) {
    _input_coord = input;
    compute_weights();
    _has_triangles = true;
  }
}

/**
 * Computes the weights for each AnimControl based on the input coordinates.
 */
void AnimChannelBlend2D::
compute_weights() {
  if (_channels.size() == 0) {
    return;
  }

  if (!_has_triangles) {
    build_triangles();
  }

  // Zero out all of the control weights to start.
  Channels::iterator ci;
  for (ci = _channels.begin(); ci != _channels.end(); ++ci) {
    (*ci)._weight = 0.0f;
  }

  LPoint2 best_point;
  bool first = true;
  int triangle = -1;
  PN_stdfloat blend_weights[3] = { 0, 0, 0 };

  for (int i = 0; i < (int)_triangles.size(); i++) {
    LPoint2 points[3];
    points[0] = _channels[_triangles[i].a]._point;
    points[1] = _channels[_triangles[i].b]._point;
    points[2] = _channels[_triangles[i].c]._point;

    if (point_in_triangle(points[0], points[1], points[2], _input_coord)) {
      triangle = i;
      blend_triangle(points[0], points[1], points[2], _input_coord, blend_weights);
      break;
    }

    for (int j = 0; j < 3; j++) {
      const LPoint2 &a = points[j];
      const LPoint2 &b = points[(j + 1) % 3];
      LPoint2 closest2 = closest_point_to_segment(_input_coord, a, b);

      if (first || (_input_coord - closest2).length() < (_input_coord - best_point).length()) {
        best_point = closest2;
        triangle = i;
        first = false;
        PN_stdfloat d = (b - a).length();
        if (d == 0.0f) {
          blend_weights[j] = 1.0f;
          blend_weights[(j + 1) % 3] = 0.0f;
          blend_weights[(j + 2) % 3] = 0.0f;

        } else {
          PN_stdfloat c = (closest2 - a).length() / d;

          blend_weights[j] = 1.0f - c;
          blend_weights[(j + 1) % 3] = c;
          blend_weights[(j + 2) % 3] = 0.0f;
        }
      }
    }
  }

  nassertv(triangle != -1);

  _active_tri = &_triangles[triangle];

  // Now apply the blend weights to the three controls in effect.
  _channels[_triangles[triangle].a]._weight = blend_weights[0];
  _channels[_triangles[triangle].b]._weight = blend_weights[1];
  _channels[_triangles[triangle].c]._weight = blend_weights[2];
}

/**
 *
 */
void AnimChannelBlend2D::
blend_triangle(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c,
               const LPoint2 &point, PN_stdfloat *weights) {
  if ((a - point).length_squared() < equal_epsilon) {
    // Close enough to be fully in point A.
    weights[0] = 1.0f;
    weights[1] = weights[2] = 0.0f;
    return;
  }

  if ((b - point).length_squared() < equal_epsilon) {
    // Close enough to be fully in point B.
    weights[1] = 1.0f;
    weights[0] = weights[2] = 0.0f;
  }

  if ((c - point).length_squared() < equal_epsilon) {
    // Close enough to be fully in point C.
    weights[2] = 1.0f;
    weights[0] = weights[1] = 0.0f;
    return;
  }

  // Need to blend between the points.
  LVector2 v0 = b - a;
  LVector2 v1 = c - a;
  LVector2 v2 = point - a;

  PN_stdfloat d00 = v0.dot(v0);
  PN_stdfloat d01 = v0.dot(v1);
  PN_stdfloat d11 = v1.dot(v1);
  PN_stdfloat d20 = v2.dot(v0);
  PN_stdfloat d21 = v2.dot(v1);
  PN_stdfloat denom = (d00 * d11 - d01 * d01);

  if (denom < equal_epsilon) {
    weights[0] = 1.0f;
    weights[1] = weights[2] = 0.0f;
    return;
  }

  PN_stdfloat oo_denom = 1.0f / denom;

  PN_stdfloat v = (d11 * d20 - d01 * d21) * oo_denom;
  PN_stdfloat w = (d00 * d21 - d01 * d20) * oo_denom;
  PN_stdfloat u = 1.0f - v - w;

  weights[0] = u;
  weights[1] = v;
  weights[2] = w;
}

/**
 * Returns true if point lies within the triangle defined by the points a, b
 * and c.
 */
bool AnimChannelBlend2D::
point_in_triangle(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c,
                  const LPoint2 &point) const {
  bool b1 = triangle_sign(point, a, b) < 0.0f;
  bool b2 = triangle_sign(point, b, c) < 0.0f;
  bool b3 = triangle_sign(point, c, a) < 0.0f;

  return ((b1 == b2) && (b2 == b3));
}

/**
 *
 */
PN_stdfloat AnimChannelBlend2D::
triangle_sign(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c) const {
  return (a[0] - c[0]) * (b[1] - c[1]) - (b[0] - c[0]) * (a[1] - c[1]);
}

/**
 *
 */
LPoint2 AnimChannelBlend2D::
closest_point_to_segment(const LPoint2 &point, const LPoint2 &a, const LPoint2 &b) const {
  LVector2 p = point - a;
  LVector2 n = b - a;
  PN_stdfloat l2 = n.length_squared();
  if (l2 < 1e-20) {
    return a;
  }

  PN_stdfloat d = n.dot(p) / l2;

  if (d <= 0.0f) {
    return a;

  } else if (d >= 1.0f) {
    return b;

  } else {
    return a + n * d;
  }
}

/**
 *
 */
PT(AnimChannel) AnimChannelBlend2D::
make_copy() const {
  return new AnimChannelBlend2D(*this);
}

/**
 * Returns the duration of the channel in the context of the indicated
 * character.
 */
PN_stdfloat AnimChannelBlend2D::
get_length(Character *character) const {
  ((AnimChannelBlend2D *)this)->compute_weights_if_necessary(character);

  if (_active_tri == nullptr) {
    return 0.0f;
  }

  const Channel &c0 = _channels[_active_tri->a];
  const Channel &c1 = _channels[_active_tri->b];
  const Channel &c2 = _channels[_active_tri->c];

  return (c0._channel->get_length(character) * c0._weight) +
         (c1._channel->get_length(character) * c1._weight) +
         (c2._channel->get_length(character) * c2._weight);
}

/**
 *
 */
void AnimChannelBlend2D::
do_calc_pose(const AnimEvalContext &context, AnimEvalData &data) {
  compute_weights_if_necessary(context._character);

  if (_active_tri == nullptr) {
    return;
  }

  const Channel &c0 = _channels[_active_tri->a];
  const Channel &c1 = _channels[_active_tri->b];
  const Channel &c2 = _channels[_active_tri->c];

  PN_stdfloat w0 = c0._weight;
  PN_stdfloat w1 = c1._weight;
  PN_stdfloat w2 = c2._weight;

  if (w0 == 1.0f) {
    c0._channel->calc_pose(context, data);
    return;

  } else if (w1 == 1.0f) {
    c1._channel->calc_pose(context, data);
    return;

  } else if (w2 == 1.0f) {
    c2._channel->calc_pose(context, data);
    return;
  }

  float orig_weight = data._weight;
  data._weight = 1.0f;
  c0._channel->calc_pose(context, data);
  data._weight = orig_weight;

  AnimEvalData c1_data;
  c1_data._weight = 1.0f;
  c1_data._cycle = data._cycle;
  c1._channel->calc_pose(context, c1_data);

  AnimEvalData c2_data;
  c2_data._weight = 1.0f;
  c2_data._cycle = data._cycle;
  c2._channel->calc_pose(context, c2_data);

  SIMDFloatVector vw0 = w0;
  SIMDFloatVector vw1 = w1;
  SIMDFloatVector vw2 = w2;

  for (int i = 0; i < context._num_joint_groups; ++i) {
    data._pose[i].pos *= vw0;
    data._pose[i].pos.madd_in_place(c1_data._pose[i].pos, vw1);
    data._pose[i].pos.madd_in_place(c2_data._pose[i].pos, vw2);

    data._pose[i].scale *= vw0;
    data._pose[i].scale.madd_in_place(c1_data._pose[i].scale, vw1);
    data._pose[i].scale.madd_in_place(c2_data._pose[i].scale, vw2);

    data._pose[i].shear *= vw0;
    data._pose[i].shear.madd_in_place(c1_data._pose[i].shear, vw1);
    data._pose[i].shear.madd_in_place(c2_data._pose[i].shear, vw2);
  }

  SIMDFloatVector diagonal_weight;

  // Blend rotation.
  if (w1 < 0.001f) {
    diagonal_weight = vw2 / (vw0 + vw2);
    for (int i = 0; i < context._num_joint_groups; ++i) {
      data._pose[i].quat = data._pose[i].quat.align_lerp(c2_data._pose[i].quat, diagonal_weight);
    }
  } else {
    diagonal_weight = vw1 / (vw0 + vw1);
    for (int i = 0; i < context._num_joint_groups; ++i) {
      data._pose[i].quat = data._pose[i].quat.align_lerp(c1_data._pose[i].quat, diagonal_weight);
      data._pose[i].quat = data._pose[i].quat.align_lerp(c2_data._pose[i].quat, vw2);
    }
  }
}

/**
 *
 */
LVector3 AnimChannelBlend2D::
get_root_motion_vector(Character *character) const {
  ((AnimChannelBlend2D *)this)->compute_weights_if_necessary(character);

  if (_active_tri == nullptr) {
    return LVector3(0.0f);
  }

  const Channel &c0 = _channels[_active_tri->a];
  const Channel &c1 = _channels[_active_tri->b];
  const Channel &c2 = _channels[_active_tri->c];

  return (c0._channel->get_root_motion_vector(character) * c0._weight) +
         (c1._channel->get_root_motion_vector(character) * c1._weight) +
         (c2._channel->get_root_motion_vector(character) * c2._weight);
}

/**
 *
 */
AnimChannelBlend2D::
AnimChannelBlend2D(const AnimChannelBlend2D &copy) :
  AnimChannel(copy),
  _blend_x(copy._blend_x),
  _blend_y(copy._blend_y),
  _channels(copy._channels),
  _triangles(copy._triangles),
  _has_triangles(copy._has_triangles),
  _active_tri(copy._active_tri),
  _input_coord(copy._input_coord)
{
}

/**
 *
 */
void AnimChannelBlend2D::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(_type_handle, make_from_bam);
}

/**
 *
 */
TypedWritable *AnimChannelBlend2D::
make_from_bam(const FactoryParams &params) {
  AnimChannelBlend2D *chan = new AnimChannelBlend2D("");
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  chan->fillin(scan, manager);
  return chan;
}

/**
 *
 */
void AnimChannelBlend2D::
write_datagram(BamWriter *manager, Datagram &me) {
  AnimChannel::write_datagram(manager, me);

  me.add_int16(_blend_x);
  me.add_int16(_blend_y);
  me.add_bool(_has_triangles);

  me.add_uint8(_channels.size());
  for (size_t i = 0; i < _channels.size(); i++) {
    const Channel *chan = &_channels[i];
    chan->_point.write_datagram(me);
    manager->write_pointer(me, chan->_channel);
  }

  me.add_uint8(_triangles.size());
  for (size_t i = 0; i < _triangles.size(); i++) {
    const Triangle *tri = &_triangles[i];
    me.add_uint8(tri->a);
    me.add_uint8(tri->b);
    me.add_uint8(tri->c);
  }
}

/**
 *
 */
int AnimChannelBlend2D::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = AnimChannel::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _channels.size(); i++) {
    _channels[i]._channel = DCAST(AnimChannel, p_list[pi++]);
  }

  return pi;
}

/**
 *
 */
void AnimChannelBlend2D::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannel::fillin(scan, manager);

  _blend_x = scan.get_int16();
  _blend_y = scan.get_int16();
  _has_triangles = scan.get_bool();

  _channels.resize(scan.get_uint8());
  for (size_t i = 0; i < _channels.size(); i++) {
    _channels[i]._point.read_datagram(scan);
    manager->read_pointer(scan);
  }

  _triangles.resize(scan.get_uint8());
  for (size_t i = 0; i < _triangles.size(); i++) {
    _triangles[i].a = scan.get_uint8();
    _triangles[i].b = scan.get_uint8();
    _triangles[i].c = scan.get_uint8();
  }
}
