/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannel.cxx
 * @author brian
 * @date 2021-08-04
 */

#include "animChannel.h"
#include "config_anim.h"
#include "mathutil_misc.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "clockObject.h"
#include "character.h"
#include "ikSolver.h"
#include "loader.h"
#include "configVariableBool.h"
#include "ikHelper.h"

static ConfigVariableBool ik_enable("ik-enable", true);

IMPLEMENT_CLASS(AnimChannel);

// For some reason delta animations have a 90 degree rotation on the root
// joint.  This quaternion reverses that.
static LQuaternion root_delta_fixup = LQuaternion(0.707107, 0, 0, 0.707107);

/**
 * Scales the quaternion rotation by the indicated amount and stores the result
 * in q.
 */
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

/**
 * Multiplies quaternion p by q and stores the result in qt.  Aligns q to p
 * before multiplying.  Uses the Source multiplication method if the config
 * variable is set.
 */
void
quaternion_mult_seq(const LQuaternion &p, const LQuaternion &q, LQuaternion &qt) {
  if (&p == &qt) {
    LQuaternion p2 = p;
    quaternion_mult_seq(p2, q, qt);
    return;
  }

  LQuaternion q2;
  LQuaternion::align(p, q, q2);

  if (source_delta_anims) {
    // Method of quaternion multiplication taken from Source engine, needed to
    // correctly layer delta animations decompiled from Source.
    qt[1] =  p[1] * q2[0] + p[2] * q2[3] - p[3] * q2[2] + p[0] * q2[1];
    qt[2] = -p[1] * q2[3] + p[2] * q2[0] + p[3] * q2[1] + p[0] * q2[2];
    qt[3] =  p[1] * q2[2] - p[2] * q2[1] + p[3] * q2[0] + p[0] * q2[3];
    qt[0] = -p[1] * q2[1] - p[2] * q2[2] - p[3] * q2[3] + p[0] * q2[0];

  } else {
    qt = p * q2;
  }
}

/**
 * Accumulates quaternion q onto p with weight s, and stores the result in qt.
 */
void
quaternion_ma_seq(const LQuaternion &p, float s, const LQuaternion &q, LQuaternion &qt) {
  LQuaternion p1, q1;

  quaternion_scale_seq(q, s, q1);
  quaternion_mult_seq(p, q1, p1);
  p1.normalize();

  qt = p1;
}

/**
 *
 */
AnimChannel::
AnimChannel(const std::string &name) :
  Namable(name),
  _flags(F_none),
  _weights(nullptr),
  _num_frames(1),
  _fps(24.0f),
  _fade_in(0.2f),
  _fade_out(0.2f)
{
}

/**
 *
 */
AnimChannel::
AnimChannel(const AnimChannel &copy) :
  Namable(copy),
  _flags(copy._flags),
  _weights(copy._weights),
  _num_frames(copy._num_frames),
  _fps(copy._fps),
  _fade_in(copy._fade_in),
  _fade_out(copy._fade_out),
  _activities(copy._activities)
{
}

/**
 *
 */
AnimChannel::
AnimChannel() :
  Namable(""),
  _flags(F_none),
  _weights(nullptr),
  _num_frames(1),
  _fps(24.0f),
  _fade_in(0.2f),
  _fade_out(0.2f)
{
}

/**
 * Adds a new event that should occur at the indicated point in this
 * AnimChannel's timeline.
 */
void AnimChannel::
add_event(int type, int event, PN_stdfloat frame, const std::string &options) {
  Event ev(type, event, frame / std::max(1.0f, _num_frames - 1.0f), options);
  _events.push_back(std::move(ev));
}

/**
 * Adds a new IK event to the channel for the indicated IK chain.
 */
void AnimChannel::
add_ik_event(const IKEvent &event) {
  IKEvent cpy = event;
  if (cpy._pose_parameter == -1) {
    // If blend driven by animation cycle, convert frame numbers to cycle
    // values.
    PN_stdfloat num_frames = std::max(1.0f, _num_frames - 1.0f);
    cpy._start /= num_frames;
    cpy._peak /= num_frames;
    cpy._tail /= num_frames;
    cpy._end /= num_frames;
  }
  _ik_events.push_back(std::move(cpy));
}

/**
 * Returns an array filled with 1s, for each possible joint.
 */
static float *
get_ident_joint_weights() {
  static float *ident_joint_weights = nullptr;
  if (ident_joint_weights == nullptr) {
    ident_joint_weights = (float *)PANDA_MALLOC_ARRAY(sizeof(float) * max_character_joints);
    for (int i = 0; i < max_character_joints; ++i) {
      ident_joint_weights[i] = 1.0f;
    }
  }
  return ident_joint_weights;
}

/**
 * Returns an array of quaternions that offset the rotation of each joint
 * in a delta animation.  Every joint but the root is set to the identity
 * quat.
 */
static SIMDQuaternionf *
get_delta_joint_offsets() {
  static SIMDQuaternionf *delta_joint_offsets = nullptr;
  if (delta_joint_offsets == nullptr) {
    delta_joint_offsets = new SIMDQuaternionf[max_character_joints / SIMDQuaternionf::num_quats];
    for (int i = 0; i < (max_character_joints / SIMDQuaternionf::num_quats); ++i) {
      delta_joint_offsets[i] = SIMDQuaternionf(LQuaternionf::ident_quat());
    }
    delta_joint_offsets[0].set_lquat(0, root_delta_fixup);
  }
  return delta_joint_offsets;
}

/**
 * Blends between "a" and "b" using the indicated weight, and stores the
 * result in "a".  A weight of 0 returns "a", 1 returns "b".  The joint
 * weights of the channel are taken into account as well.  "b" may be
 * invalidated after calling this method, so don't try to access its joints
 * after calling this method.
 */
void AnimChannel::
blend(const AnimEvalContext &context, AnimEvalData &a,
      AnimEvalData &b, PN_stdfloat weight) const {

  weight = std::clamp(weight, 0.0f, 1.0f);

  if (weight == 0.0f) {
    return;

  } else if (_weights == nullptr && weight == 1.0f && !has_flags(F_delta | F_pre_delta)) {
    // If there's no per-joint weight list, the blend has full weight on B, and
    // we're not an additive channel, just move B to A.
    a.steal_pose(b, context._num_joint_groups);
    return;
  }

  SIMDFloatVector vweight = weight;
  SIMDFloatVector *vweights;
  if (_weights != nullptr) {
    vweights = reinterpret_cast<SIMDFloatVector *>(_weights->get_weights());
  } else {
    vweights = reinterpret_cast<SIMDFloatVector *>(get_ident_joint_weights());
  }

  if (has_flags(F_delta | F_pre_delta)) {
    // Additive blend.

    SIMDQuaternionf *delta_offsets = get_delta_joint_offsets();

    for (int i = 0; i < context._num_joint_groups; i++) {
      SIMDFloatVector s2 = vweight * vweights[i];

      a._pose[i].pos.madd_in_place(b._pose[i].pos, s2);
      a._pose[i].quat = a._pose[i].quat.accumulate_scaled_rhs_source(b._pose[i].quat * delta_offsets[i], s2);
    }

    // Blend in sliders.
    for (int i = 0; i < context._num_slider_groups; ++i) {
      a._sliders[i].madd_in_place(b._sliders[i], vweight);
    }

  } else {
    SIMDFloatVector v1(1.0f);

    // Mix blend.
    for (int i = 0; i < context._num_joint_groups; i++) {
      SIMDFloatVector s2 = vweight * vweights[i];
      SIMDFloatVector s1 = v1 - s2;

      a._pose[i].pos *= s1;
      a._pose[i].pos.madd_in_place(b._pose[i].pos, s2);

      a._pose[i].scale *= s1;
      a._pose[i].scale.madd_in_place(b._pose[i].scale, s2);

      a._pose[i].shear *= s1;
      a._pose[i].shear.madd_in_place(b._pose[i].shear, s2);

      a._pose[i].quat = a._pose[i].quat.align_lerp(b._pose[i].quat, s2);
    }

    // Blend in sliders.
    for (int i = 0; i < context._num_slider_groups; ++i) {
      SIMDFloatVector s1 = v1 - vweight;
      a._sliders[i] *= s1;
      a._sliders[i].madd_in_place(b._sliders[i], vweight);
    }
  }
}

/**
 *
 */
void AnimChannel::
calc_pose(const AnimEvalContext &context, AnimEvalData &data) {
  if (data._weight == 0.0f) {
    return;
  }

  AnimEvalData this_data(data, context._num_joint_groups);

  if (has_flags(F_real_time)) {
    // Compute cycle from current rendering time instead of relative to
    // the start time of the sequence.
    PN_stdfloat cps = get_cycle_rate(context._character);
    this_data._cycle = ClockObject::get_global_clock()->get_frame_time() * cps;
    this_data._cycle -= (int)this_data._cycle;
  }

  // Implementation-specific pose calculation.
  do_calc_pose(context, this_data);

  // Zero out requested root translational axes.  This is done when a
  // locomotion animation has movement part of the root joint of the
  // animation, but the character needs to remain stationary so it can be
  // moved around with game code.
  if (has_flags(F_zero_root_x)) {
    //this_data._pose[0]._position[0] = 0.0f;
  }
  if (has_flags(F_zero_root_y)) {
    //this_data._pose[0]._position[1] = 0.0f;
  }
  if (has_flags(F_zero_root_z)) {
    //this_data._pose[0]._position[2] = 0.0f;
  }

  if (ik_enable && context._ik != nullptr) {
    // Add global IK events from this channel, such as touches.
    context._ik->add_channel_events(this, data);
  }

  // Do a local IK pass for lock events.
  IKHelper local_ik(&context, true);
  if (ik_enable) {
    local_ik.add_channel_events(this, data);
  }

  // Now blend the channel onto the output using the requested weight.
  blend(context, data, this_data, data._weight);

  if (ik_enable) {
    // Apply local IK events, such as locks.
    local_ik.apply_ik(data, data._weight);
  }
}

/**
 *
 */
int AnimChannel::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  _weights = DCAST(WeightList, p_list[pi++]);

  return pi;
}

/**
 * Writes the object to the indicated Datagram for shipping out to a Bam file.
 */
void AnimChannel::
write_datagram(BamWriter *manager, Datagram &me) {
  TypedWritableReferenceCount::write_datagram(manager, me);

  me.add_string(get_name());

  me.add_uint16(_num_frames);
  me.add_stdfloat(_fps);

  me.add_uint32(_flags);

  me.add_uint8(_activities.size());
  for (size_t i = 0; i < _activities.size(); i++) {
    me.add_uint32(_activities[i].activity);
    me.add_stdfloat(_activities[i].weight);
  }

  me.add_stdfloat(_fade_in);
  me.add_stdfloat(_fade_out);

  me.add_uint8(_events.size());
  for (size_t i = 0; i < _events.size(); i++) {
    me.add_uint8(_events[i]._type);
    me.add_stdfloat(_events[i]._cycle);
    me.add_int16(_events[i]._event);
    me.add_string(_events[i]._options);
  }

  me.add_uint8(_ik_events.size());
  for (size_t i = 0; i < _ik_events.size(); ++i) {
    const IKEvent &event = _ik_events[i];
    me.add_int8(event._type);
    me.add_int8(event._chain);
    me.add_int16(event._touch_joint);
    me.add_stdfloat(event._start);
    me.add_stdfloat(event._peak);
    me.add_stdfloat(event._tail);
    me.add_stdfloat(event._end);
    me.add_bool(event._spline);
    me.add_int8(event._pose_parameter);
    if (event._type == IKEvent::T_touch) {
      me.add_uint16(event._touch_offsets.size());
      for (size_t j = 0; j < event._touch_offsets.size(); ++j) {
        event._touch_offsets[j]._pos.write_datagram(me);
        event._touch_offsets[j]._hpr.write_datagram(me);
      }
    }
  }

  manager->write_pointer(me, _weights);
}

/**
 * Reads in the object from the indicated Datagram.
 */
void AnimChannel::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  set_name(scan.get_string());

  _num_frames = (int)scan.get_uint16();
  _fps = scan.get_stdfloat();

  _flags = scan.get_uint32();

  _activities.resize(scan.get_uint8());
  for (size_t i = 0; i < _activities.size(); i++) {
    _activities[i].activity = scan.get_uint32();
    _activities[i].weight = scan.get_stdfloat();
  }

  _fade_in = scan.get_stdfloat();
  _fade_out = scan.get_stdfloat();

  _events.resize(scan.get_uint8());
  for (size_t i = 0; i < _events.size(); i++) {
    _events[i]._type = scan.get_uint8();
    _events[i]._cycle = scan.get_stdfloat();
    _events[i]._event = scan.get_int16();
    _events[i]._options = scan.get_string();
  }

  _ik_events.resize(scan.get_uint8());
  for (size_t i = 0; i < _ik_events.size(); ++i) {
    IKEvent &event = _ik_events[i];
    event._type = (IKEvent::Type)scan.get_int8();
    event._chain = scan.get_int8();
    event._touch_joint = scan.get_int16();
    event._start = scan.get_stdfloat();
    event._peak = scan.get_stdfloat();
    event._tail = scan.get_stdfloat();
    event._end = scan.get_stdfloat();
    event._spline = scan.get_bool();
    event._pose_parameter = scan.get_int8();
    if (event._type == IKEvent::T_touch) {
      event._touch_offsets.resize(scan.get_uint16());
      for (size_t j = 0; j < event._touch_offsets.size(); ++j) {
        event._touch_offsets[j]._pos.read_datagram(scan);
        event._touch_offsets[j]._hpr.read_datagram(scan);
      }
    }
  }

  manager->read_pointer(scan); // _weights
}
