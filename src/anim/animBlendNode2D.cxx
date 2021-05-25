/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBlendNode2D.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animBlendNode2D.h"
#include "triangulatorDelaunay.h"
#include "poseParameter.h"
#include "character.h"

static const PN_stdfloat equal_epsilon = 0.001f;

TypeHandle AnimBlendNode2D::_type_handle;

/**
 *
 */
AnimBlendNode2D::
AnimBlendNode2D(const std::string &name) :
  AnimGraphNode(name),
  _has_triangles(false),
  _active_tri(nullptr),
  _x_param(-1),
  _y_param(-1)
{
}

/**
 * Builds a set of triangles out of all input points.
 */
void AnimBlendNode2D::
build_triangles() {
  _triangles.clear();

  TriangulatorDelaunay triangles;
  for (size_t i = 0; i < _inputs.size(); i++) {
    triangles.add_point(_inputs[i]._point);
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
 * Computes the weights for each AnimControl based on the input coordinates.
 */
void AnimBlendNode2D::
compute_weights() {
  if (_inputs.size() == 0) {
    return;
  }

  if (!_has_triangles) {
    build_triangles();
  }

  // Zero out all of the control weights to start.
  Inputs::iterator ci;
  for (ci = _inputs.begin(); ci != _inputs.end(); ++ci) {
    (*ci)._weight = 0.0f;
  }

  LPoint2 best_point;
  bool first = true;
  int triangle = -1;
  PN_stdfloat blend_weights[3] = { 0, 0, 0 };

  for (int i = 0; i < (int)_triangles.size(); i++) {
    LPoint2 points[3];
    points[0] = _inputs[_triangles[i].a]._point;
    points[1] = _inputs[_triangles[i].b]._point;
    points[2] = _inputs[_triangles[i].c]._point;

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
  _inputs[_triangles[triangle].a]._weight = blend_weights[0];
  _inputs[_triangles[triangle].b]._weight = blend_weights[1];
  _inputs[_triangles[triangle].c]._weight = blend_weights[2];
}

/**
 * Builds triangles and computes weights if something changed.
 */
void AnimBlendNode2D::
compute_weights_if_necessary(Character *character) {
  LPoint2 input(0);
  if (_x_param != -1) {
    input[0] = character->get_pose_parameter(_x_param).get_value();
  }
  if (_y_param != -1) {
    input[1] = character->get_pose_parameter(_y_param).get_value();
  }
  if ((input != _input_coord) || !_has_triangles) {
    _input_coord = input;
    compute_weights();
    _has_triangles = true;
  }
}

/**
 *
 */
void AnimBlendNode2D::
evaluate(AnimGraphEvalContext &context) {
  compute_weights_if_necessary(context._character);

  if (_active_tri == nullptr) {
    return;
  }

  AnimGraphNode *i0 = _inputs[_active_tri->a]._node;
  AnimGraphNode *i1 = _inputs[_active_tri->b]._node;
  AnimGraphNode *i2 = _inputs[_active_tri->c]._node;

  PN_stdfloat w0 = _inputs[_active_tri->a]._weight;
  PN_stdfloat w1 = _inputs[_active_tri->b]._weight;
  PN_stdfloat w2 = _inputs[_active_tri->c]._weight;

  AnimGraphEvalContext i0_ctx(context);
  AnimGraphEvalContext i1_ctx(context);
  AnimGraphEvalContext i2_ctx(context);

  if (w0 != 0.0f) {
    i0->evaluate(i0_ctx);
  }

  if (w1 != 0.0f) {
    i1->evaluate(i1_ctx);
  }

  if (w2 != 0.0f) {
    i2->evaluate(i2_ctx);
  }

  if (w0 == 1.0f) {
    context.steal(i0_ctx);

  } else if (w1 == 1.0f) {
    context.steal(i1_ctx);

  } else if (w2 == 1.0f) {
    context.steal(i2_ctx);

  } else {
    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &joint = context._joints[i];
      joint._position.set(0, 0, 0);
      joint._scale.set(0, 0, 0);
      JointTransform &a_joint = i0_ctx._joints[i];
      JointTransform &b_joint = i1_ctx._joints[i];
      JointTransform &c_joint = i2_ctx._joints[i];

      if (w0 != 0.0f) {
        joint._position += a_joint._position * w0;
        joint._scale += a_joint._scale * w0;
      }
      if (w1 != 0.0f) {
        joint._position += b_joint._position * w1;
        joint._scale += b_joint._scale * w1;
      }
      if (w2 != 0.0f) {
        joint._position += c_joint._position * w2;
        joint._scale += c_joint._scale * w2;
      }

      if (w1 < 0.001f) {
        // On diagonal.
        LQuaternion::blend(a_joint._rotation, c_joint._rotation, w2 / (w0 + w2), joint._rotation);
      } else {
        LQuaternion q;
        LQuaternion::blend(a_joint._rotation, b_joint._rotation, w1 / (w0 + w1), q);
        LQuaternion::blend(q, c_joint._rotation, w2, joint._rotation);
      }
    }
  }
}

/**
 *
 */
void AnimBlendNode2D::
evaluate_anims(pvector<AnimBundle *> &anims, vector_stdfloat &weights, PN_stdfloat this_weight) {
  //compute_weights_if_necessary();

  if (_active_tri == nullptr) {
    return;
  }

  AnimGraphNode *i0 = _inputs[_active_tri->a]._node;
  AnimGraphNode *i1 = _inputs[_active_tri->b]._node;
  AnimGraphNode *i2 = _inputs[_active_tri->c]._node;

  PN_stdfloat w0 = _inputs[_active_tri->a]._weight;
  PN_stdfloat w1 = _inputs[_active_tri->b]._weight;
  PN_stdfloat w2 = _inputs[_active_tri->c]._weight;

  i0->evaluate_anims(anims, weights, this_weight * w0);
  i1->evaluate_anims(anims, weights, this_weight * w1);
  i2->evaluate_anims(anims, weights, this_weight * w2);
}

/**
 *
 */
void AnimBlendNode2D::
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
bool AnimBlendNode2D::
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
PN_stdfloat AnimBlendNode2D::
triangle_sign(const LPoint2 &a, const LPoint2 &b, const LPoint2 &c) const {
  return (a[0] - c[0]) * (b[1] - c[1]) - (b[0] - c[0]) * (a[1] - c[1]);
}

/**
 *
 */
LPoint2 AnimBlendNode2D::
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
