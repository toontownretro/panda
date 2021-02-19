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

static const PN_stdfloat equal_epsilon = 0.001f;

TypeHandle AnimBlendNode2D::_type_handle;

/**
 *
 */
AnimBlendNode2D::
AnimBlendNode2D(const std::string &name) :
  AnimGraphNode(name),
  _has_triangles(false),
  _input_coord_changed(true),
  _active_tri(nullptr)
{
}

/**
 * Builds a set of triangles out of all input points.
 */
void AnimBlendNode2D::
build_triangles() {
  _triangles.clear();

  TriangulatorDelaunay triangles;
  for (size_t i = 0; i < _input_points.size(); i++) {
    triangles.add_point(_input_points[i]);
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
  if (_input_points.size() == 0) {
    return;
  }

  if (!_has_triangles) {
    build_triangles();
  }

  // Zero out all of the control weights to start.
  vector_stdfloat::iterator ci;
  for (ci = _input_weights.begin(); ci != _input_weights.end(); ++ci) {
    (*ci) = 0.0f;
  }

  LPoint2 best_point;
  bool first = true;
  int triangle = -1;
  PN_stdfloat blend_weights[3] = { 0, 0, 0 };

  for (int i = 0; i < (int)_triangles.size(); i++) {
    LPoint2 points[3];
    points[0] = _input_points[_triangles[i].a];
    points[1] = _input_points[_triangles[i].b];
    points[2] = _input_points[_triangles[i].c];

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
  _input_weights[_triangles[triangle].a] = blend_weights[0];
  _input_weights[_triangles[triangle].b] = blend_weights[1];
  _input_weights[_triangles[triangle].c] = blend_weights[2];
}

/**
 *
 */
void AnimBlendNode2D::
evaluate(MovingPartMatrix *part, bool frame_blend_flag) {
  if (_input_coord_changed || !_has_triangles) {
    compute_weights();
    _input_coord_changed = false;
    _has_triangles = true;
  }

  if (_active_tri == nullptr) {
    return;
  }

  AnimGraphNode *i0 = _inputs[_active_tri->a];
  AnimGraphNode *i1 = _inputs[_active_tri->b];
  AnimGraphNode *i2 = _inputs[_active_tri->c];

  PN_stdfloat w0 = _input_weights[_active_tri->a];
  PN_stdfloat w1 = _input_weights[_active_tri->b];
  PN_stdfloat w2 = _input_weights[_active_tri->c];

  if (w0 != 0.0f) {
    i0->evaluate(part, frame_blend_flag);
  }

  if (w1 != 0.0f) {
    i1->evaluate(part, frame_blend_flag);
  }

  if (w2 != 0.0f) {
    i2->evaluate(part, frame_blend_flag);
  }

  if (w0 == 1.0f) {
    _position = i0->get_position();
    _rotation = i0->get_rotation();
    _scale = i0->get_scale();
    _shear = i0->get_shear();

  } else if (w1 == 1.0f) {
    _position = i1->get_position();
    _rotation = i1->get_rotation();
    _scale = i1->get_scale();
    _shear = i1->get_shear();

  } else if (w2 == 1.0f) {
    _position = i2->get_position();
    _rotation = i2->get_rotation();
    _scale = i2->get_scale();
    _shear = i2->get_shear();

  } else {
    _position.set(0, 0, 0);
    _rotation.set(0, 0, 0, 0);
    _scale.set(0, 0, 0);
    _shear.set(0, 0, 0);

    if (w0 != 0.0f) {
      _position += i0->get_position() * w0;
      _scale += i0->get_scale() * w0;
      _shear += i0->get_shear() * w0;
    }
    if (w1 != 0.0f) {
      _position += i1->get_position() * w1;
      _scale += i1->get_scale() * w1;
      _shear += i1->get_shear() * w1;
    }
    if (w2 != 0.0f) {
      _position += i2->get_position() * w2;
      _scale += i2->get_scale() * w2;
      _shear += i2->get_shear() * w2;
    }

    if (w1 < 0.001f) {
      // On diagonal.
      LQuaternion::blend(i0->get_rotation(), i2->get_rotation(), w2 / (w0 + w2), _rotation);
    } else {
      LQuaternion q;
      LQuaternion::blend(i0->get_rotation(), i1->get_rotation(), w1 / (w0 + w1), q);
      LQuaternion::blend(q, i2->get_rotation(), w2, _rotation);
    }
  }
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
