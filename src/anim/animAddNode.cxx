/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animAddNode.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animAddNode.h"

TypeHandle AnimAddNode::_type_handle;

void
quaternion_scale(const LQuaternion &p, float t, LQuaternion &q) {
  float r;

  float sinom = std::sqrt(p.get_ijk().dot(p.get_ijk()));
  sinom = std::min(sinom, 1.0f);

  float sinsom = std::sin(std::asin(sinom) * t);

  t = sinsom / (sinom + FLT_EPSILON);

  q[1] = p[1] * t;
  q[2] = p[2] * t;
  q[3] = p[3] * t;

  r = 1.0f - sinsom * sinsom;

  if (r < 0.0f) {
    r = 0.0f;
  }

  r = std::sqrt(r);

  // Keep sign of rotation
  if (p[0] < 0) {
    q[0] = -r;
  } else {
    q[0] = r;
  }
}

void
quaternion_mult(const LQuaternion &p, const LQuaternion &q, LQuaternion &qt) {
  if (&p == &qt) {
    LQuaternion p2 = p;
    quaternion_mult(p2, q, qt);
    return;
  }

  LQuaternion q2;
  LQuaternion::align(p, q, q2);

  // Method of quaternion multiplication taken from Source engine, needed to
  // correctly layer delta animations decompiled from Source.
  qt[1] =  p[1] * q2[0] + p[2] * q2[3] - p[3] * q2[2] + p[0] * q2[1];
	qt[2] = -p[1] * q2[3] + p[2] * q2[0] + p[3] * q2[1] + p[0] * q2[2];
	qt[3] =  p[1] * q2[2] - p[2] * q2[1] + p[3] * q2[0] + p[0] * q2[3];
	qt[0] = -p[1] * q2[1] - p[2] * q2[2] - p[3] * q2[3] + p[0] * q2[0];

  //qt = p * q2;
}

void
quaternion_sm(float s, const LQuaternion &p, const LQuaternion &q, LQuaternion &qt) {
  LQuaternion p1, q1;

  quaternion_scale(p, s, p1);
  quaternion_mult(p1, q, q1);
  q1.normalize();

  qt = q1;
}

void
quaternion_ma(const LQuaternion &p, float s, const LQuaternion &q, LQuaternion &qt) {
  LQuaternion p1, q1;

  quaternion_scale(q, s, q1);
  quaternion_mult(p, q1, p1);
  p1.normalize();

  qt = p1;
}

/**
 *
 */
AnimAddNode::
AnimAddNode(const std::string &name) :
  AnimGraphNode(name),
  _alpha(1.0f) {
}

/**
 *
 */
void AnimAddNode::
evaluate(AnimGraphEvalContext &context) {
  nassertv(_base != nullptr && _add != nullptr);

  if (_alpha <= 0.001f) {
    // Not doing any adding.  Fast path.
    _base->evaluate(context);

  } else {
    AnimGraphEvalContext base_ctx(context);
    _base->evaluate(base_ctx);

    AnimGraphEvalContext add_ctx(context);
    _add->evaluate(add_ctx);

    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &joint = context._joints[i];
      JointTransform &base_joint = base_ctx._joints[i];
      JointTransform &add_joint = add_ctx._joints[i];

      joint._position = base_joint._position + (add_joint._position * _alpha);
      quaternion_ma(base_joint._rotation, _alpha, add_joint._rotation, joint._rotation);

      // Not adding scale.
      joint._scale = base_joint._scale;
    }
  }
}
