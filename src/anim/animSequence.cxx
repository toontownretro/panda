/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequence.cxx
 * @author lachbr
 * @date 2021-03-01
 */

#include "animSequence.h"
#include "clockObject.h"
#include "bitArray.h"
#include "poseParameter.h"
#include "character.h"

// For some reason delta animations have a 90 degree rotation on the root
// joint.  This quaternion reverses that.
static LQuaternion root_delta_fixup = LQuaternion(0.707107, 0, 0, 0.707107);

TypeHandle AnimSequence::_type_handle;

void
quaternion_scale_seq(const LQuaternion &p, float t, LQuaternion &q) {
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
quaternion_mult_seq(const LQuaternion &p, const LQuaternion &q, LQuaternion &qt) {
  if (&p == &qt) {
    LQuaternion p2 = p;
    quaternion_mult_seq(p2, q, qt);
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
quaternion_ma_seq(const LQuaternion &p, float s, const LQuaternion &q, LQuaternion &qt) {
  LQuaternion p1, q1;

  quaternion_scale_seq(q, s, q1);
  quaternion_mult_seq(p, q1, p1);
  p1.normalize();

  qt = p1;
}

PN_stdfloat
simple_spline(PN_stdfloat s) {
  PN_stdfloat val_squared = s * s;
  return (3 * val_squared - 2 * val_squared * s);
}

/**
 *
 */
void AnimSequence::
set_frame_rate(int fps) {
  _frame_rate = fps;
  set_flags(F_frame_rate);
}

/**
 *
 */
void AnimSequence::
clear_frame_rate() {
  clear_flags(F_frame_rate);
}

/**
 *
 */
double AnimSequence::
get_frame_rate() const {
  if (has_flags(F_frame_rate)) {
    return _frame_rate;
  }

  nassertr(!_anims.empty(), 30.0);
  return _anims[0]->get_base_frame_rate();
}

/**
 *
 */
void AnimSequence::
set_num_frames(int num_frames) {
  _num_frames = num_frames;
  set_flags(F_num_frames | F_frame_rate);
}

/**
 *
 */
void AnimSequence::
clear_num_frames() {
  clear_flags(F_num_frames);
}

/**
 *
 */
int AnimSequence::
get_num_frames() const {
  if (has_flags(F_num_frames)) {
    return _num_frames;
  }

  nassertr(!_anims.empty(), 1);

  return _anims[0]->get_num_frames();
}

/**
 *
 */
PN_stdfloat AnimSequence::
get_length() {
  if (_anims.size() == 0) {
    return (get_num_frames() - 1) / get_frame_rate();
  }

  pvector<AnimBundle *> anims;
  vector_stdfloat weights;
  evaluate_anims(anims, weights);

  PN_stdfloat length = 0;
  for (size_t i = 0; i < anims.size(); i++) {
    AnimBundle *anim = anims[i];
    length += ((anim->get_num_frames() - 1) / anim->get_base_frame_rate()) * weights[i];
  }

  return length;
}

/**
 *
 */
PN_stdfloat AnimSequence::
get_cycles_per_second() {
  PN_stdfloat length = get_length();
  if (length == 0.0f) {
    return 0.0f;
  }

  return 1.0f / length;
}

/**
 *
 */
void AnimSequence::
add_event(int type, int event, int frame, const std::string &options) {
  nassertv(get_num_frames() > 1);

  PN_stdfloat cycle = (PN_stdfloat)frame / (PN_stdfloat)(get_num_frames() - 1);
  AnimEvent ae(type, event, cycle, options);
  _events.push_back(std::move(ae));
}

/**
 *
 */
void AnimSequence::
evaluate(AnimGraphEvalContext &context) {
  if (_base != nullptr) {
    AnimGraphEvalContext base_ctx(context);
    base_ctx._looping = has_flags(F_looping);
    if (has_flags(F_real_time)) {
      // Compute cycle from current rendering time instead of relative to
      // the start time of the sequence.
      PN_stdfloat cps = get_cycles_per_second();
      base_ctx._cycle = ClockObject::get_global_clock()->get_frame_time() * cps;
      base_ctx._cycle -= (int)base_ctx._cycle;
    }

    _base->evaluate(base_ctx);

    // Zero out requested root translational axes.  This is done when a
    // locomotion animation has movement part of the root joint of the
    // animation, but the character needs to remain stationary so it can be
    // moved around with game code.
    if (has_flags(F_zero_root_x)) {
      base_ctx._joints[0]._position[0] = 0.0f;
    }
    if (has_flags(F_zero_root_y)) {
      base_ctx._joints[0]._position[1] = 0.0f;
    }
    if (has_flags(F_zero_root_z)) {
      base_ctx._joints[0]._position[2] = 0.0f;
    }

    blend(context, base_ctx, context._weight);
  }

  if (_layers.empty()) {
    return;
  }

  PN_stdfloat cycle = context._cycle;
  PN_stdfloat weight = context._weight;

  // Add our layers.
  for (size_t i = 0; i < _layers.size(); i++) {
    const Layer &layer = _layers[i];

    PN_stdfloat layer_cycle = cycle;
    PN_stdfloat layer_weight = weight;

    PN_stdfloat start, peak, tail, end;

    start = layer._start;
    peak = layer._peak;
    end = layer._end;
    tail = layer._tail;

    if (start != end) {
      PN_stdfloat index;

      if (layer._pose_parameter == -1) {
        index = cycle;

      } else {
        // Layer driven by pose parameter.
        const PoseParameter &pp = context._character->get_pose_parameter(layer._pose_parameter);
        index = pp.get_value();
      }

      if (index < start || index >= end) {
        // Not in the frame range.
        continue;
      }

      PN_stdfloat scale = 1.0f;

      if (index < peak && start != peak) {
        // On the way up.
        scale = (index - start) / (peak - start);

      } else if (index > tail && end != tail) {
        // On the way down.
        scale = (end - index) / (end - tail);
      }

      if (layer._spline) {
        // Spline blend.
        scale = simple_spline(scale);
      }

      if (layer._xfade && (index > tail)) {
        layer_weight = (scale * weight) / (1 - weight + scale * weight);

      } else if (layer._no_blend) {
        layer_weight = scale;

      } else {
        layer_weight = weight * scale;
      }

      if (layer._pose_parameter == -1) {
        layer_cycle = (cycle - start) / (end - start);
      }
    }

    if (layer_weight <= 0.001f) {
      // Negligible weight.
      continue;
    }

    context._weight = layer_weight;
    context._cycle = layer_cycle;
    layer._seq->evaluate(context);
  }
}

/**
 * Initializes the joint poses of the given context for this sequence.  Sets
 * each joint to its bind pose.
 */
void AnimSequence::
init_pose(AnimGraphEvalContext &context) {
  for (int i = 0; i < context._num_joints; i++) {
    if (!context._joint_mask.get_bit(i)) {
      continue;
    }
    context._joints[i]._position = context._parts[i]._default_pos;
    context._joints[i]._rotation = context._parts[i]._default_quat;
    context._joints[i]._scale = context._parts[i]._default_scale;
  }
}

/**
 * Blends together context A with context B.  Result is stored in context A.
 * Weight of 1 returns B, 0 returns A.
 */
void AnimSequence::
blend(AnimGraphEvalContext &a, AnimGraphEvalContext &b, PN_stdfloat weight) {
  if (weight <= 0.0f) {
    return;
  }

  if (weight >= 1.0f) {
    weight = 1.0f;
  }

  int num_joints = b._num_joints;
  int i;
  PN_stdfloat s1, s2;

  // Build per-joint weight list.
  PN_stdfloat *weights = (PN_stdfloat *)alloca(num_joints * sizeof(PN_stdfloat));
  for (i = 0; i < num_joints; i++) {
    if (!b._joint_mask.get_bit(i)) {
      // Don't care about this joint.
      weights[i] = 0.0f;

    } else if (_weights != nullptr) {
      weights[i] = weight * _weights->get_weight(i);

    } else {
      weights[i] = weight;
    }
  }

  if (has_flags(F_delta)) {
    for (i = 0; i < num_joints; i++) {
      s2 = weights[i];
      if (s2 <= 0.0f) {
        continue;
      }

      if (has_flags(F_post)) {
        // Overlay delta.
        LQuaternion b_rot;
        if (i == 0) {
          // Apply the stupid rotation fix for the root joint of delta animations.
          b_rot = b._joints[i]._rotation * root_delta_fixup;
        } else {
          b_rot = b._joints[i]._rotation;
        }
        quaternion_ma_seq(a._joints[i]._rotation, s2, b_rot, a._joints[i]._rotation);
        a._joints[i]._position = a._joints[i]._position + (b._joints[i]._position * s2);
        // Not doing scale.

      } else {
        // Underlay delta.

        // FIXME: Should be quaternion SM, not implemented yet.
        quaternion_ma_seq(a._joints[i]._rotation, s2, b._joints[i]._rotation, a._joints[i]._rotation);
        a._joints[i]._position = a._joints[i]._position + (b._joints[i]._position * s2);
        // Not doing scale.
      }
    }

    return;
  }

  LQuaternion q3;
  for (i = 0; i < num_joints; i++) {
    s2 = weights[i];
    if (s2 <= 0.0f) {
      continue;
    }

    s1 = 1.0f - s2;

    //LQuaternion::blend(b._joints[i]._rotation, a._joints[i]._rotation, s1, q3);
    LQuaternion::slerp(b._joints[i]._rotation, a._joints[i]._rotation, s1, q3);

    a._joints[i]._rotation = q3;
    a._joints[i]._position = (a._joints[i]._position * s1) + (b._joints[i]._position * s2);
    a._joints[i]._scale = (a._joints[i]._scale * s1) + (b._joints[i]._scale * s2);
  }
}

/**
 *
 */
void AnimSequence::
compute_effective_control() {
  _anims.clear();

  r_compute_effective_control(this);
}

/**
 *
 */
void AnimSequence::
r_compute_effective_control(AnimGraphNode *node) {
  if (node->is_of_type(AnimBundle::get_class_type())) {
    AnimBundle *anim = DCAST(AnimBundle, node);

    _anims.push_back(anim);

    return;
  }

  for (int i = 0; i < node->get_num_children(); i++) {
    AnimGraphNode *child = node->get_child(i);
    r_compute_effective_control(child);
  }
}
