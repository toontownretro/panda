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
  _blend_y(-1)
{
}

/**
 * Builds a set of triangles out of all input points.  This must be called
 * before the channel is used on a Character.
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
 * Returns the triangle and vertex blending weights for the given Character.
 */
bool AnimChannelBlend2D::
compute_weights(Character *character, int &triangle, PN_stdfloat weights[3]) const {
  if (_channels.size() == 0) {
    return false;
  }

  nassertr(_has_triangles, false);

  LVecBase2 coord(0.0f);
  if (_blend_x != -1) {
    const PoseParameter &x = character->get_pose_parameter(_blend_x);
    coord[0] = x.get_norm_value();
  }
  if (_blend_y != -1) {
    const PoseParameter &y = character->get_pose_parameter(_blend_y);
    coord[1] = y.get_norm_value();
  }

  LPoint2 best_point;
  bool first = true;
  triangle = -1;
  weights[0] = weights[1] = weights[2] = 0.0f;

  for (int i = 0; i < (int)_triangles.size(); i++) {
    LPoint2 points[3];
    points[0] = _channels[_triangles[i].a]._point;
    points[1] = _channels[_triangles[i].b]._point;
    points[2] = _channels[_triangles[i].c]._point;

    if (point_in_triangle(points[0], points[1], points[2], coord)) {
      triangle = i;
      blend_triangle(points[0], points[1], points[2], coord, weights);
      break;
    }

    for (int j = 0; j < 3; j++) {
      const LPoint2 &a = points[j];
      const LPoint2 &b = points[(j + 1) % 3];
      LPoint2 closest2 = closest_point_to_segment(coord, a, b);

      if (first || (coord - closest2).length() < (coord - best_point).length()) {
        best_point = closest2;
        triangle = i;
        first = false;
        PN_stdfloat d = (b - a).length();
        if (d == 0.0f) {
          weights[j] = 1.0f;
          weights[(j + 1) % 3] = 0.0f;
          weights[(j + 2) % 3] = 0.0f;

        } else {
          PN_stdfloat c = (closest2 - a).length() / d;

          weights[j] = 1.0f - c;
          weights[(j + 1) % 3] = c;
          weights[(j + 2) % 3] = 0.0f;
        }
      }
    }
  }

  if (triangle == -1) {
    return false;
  }

  return true;
}

/**
 *
 */
void AnimChannelBlend2D::
blend_triangle(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c,
               const LPoint2 &point, PN_stdfloat *weights) const {
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
    return;
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
  int tri_index;
  PN_stdfloat weights[3];
  if (!compute_weights(character, tri_index, weights)) {
    return 0.01f;
  }

  const Triangle *tri = &_triangles[tri_index];

  const Channel &c0 = _channels[tri->a];
  const Channel &c1 = _channels[tri->b];
  const Channel &c2 = _channels[tri->c];

  return (c0._channel->get_length(character) * weights[0]) +
         (c1._channel->get_length(character) * weights[1]) +
         (c2._channel->get_length(character) * weights[2]);
}

/**
 *
 */
void AnimChannelBlend2D::
do_calc_pose(const AnimEvalContext &context, AnimEvalData &data) {
  int tri_index;
  PN_stdfloat weights[3];
  if (!compute_weights(context._character, tri_index, weights)) {
    return;
  }

  const Triangle *tri = &_triangles[tri_index];

  const Channel &c0 = _channels[tri->a];
  const Channel &c1 = _channels[tri->b];
  const Channel &c2 = _channels[tri->c];

  PN_stdfloat w0 = weights[0];
  PN_stdfloat w1 = weights[1];
  PN_stdfloat w2 = weights[2];

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

  PN_stdfloat this_net_weight = data._net_weight;

  float orig_weight = data._weight;
  data._weight = 1.0f;
  data._net_weight = this_net_weight * w0;
  c0._channel->calc_pose(context, data);
  data._weight = orig_weight;

  AnimEvalData c1_data;
  c1_data._weight = 1.0f;
  c1_data._cycle = data._cycle;
  c1_data._net_weight = this_net_weight * w1;
  c1._channel->calc_pose(context, c1_data);

  AnimEvalData c2_data;
  c2_data._weight = 1.0f;
  c2_data._cycle = data._cycle;
  c2_data._net_weight = this_net_weight * w2;
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

  data._net_weight = this_net_weight;
}

/**
 *
 */
LVector3 AnimChannelBlend2D::
get_root_motion_vector(Character *character) const {
  int tri_index;
  PN_stdfloat weights[3];
  if (!compute_weights(character, tri_index, weights)) {
    return 0.01f;
  }

  const Triangle *tri = &_triangles[tri_index];

  const Channel &c0 = _channels[tri->a];
  const Channel &c1 = _channels[tri->b];
  const Channel &c2 = _channels[tri->c];

  return (c0._channel->get_root_motion_vector(character) * weights[0]) +
         (c1._channel->get_root_motion_vector(character) * weights[1]) +
         (c2._channel->get_root_motion_vector(character) * weights[2]);
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
  _has_triangles(copy._has_triangles)
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

  if (!_has_triangles) {
    build_triangles();
  }
}
