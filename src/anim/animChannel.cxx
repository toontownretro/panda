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
    // we're not an additive channel, just copy B to A.
    a.steal_pose(b, context._num_joints);
    return;
  }

  int num_joints = context._num_joints;
  int i;
  PN_stdfloat s1, s2;

  // Build per-joint weight list.
  PN_stdfloat *weights = (PN_stdfloat *)alloca(num_joints * sizeof(PN_stdfloat));
  for (i = 0; i < num_joints; i++) {
    if (!CheckBit(context._joint_mask, i)) {
      // Don't care about this joint.
      weights[i] = 0.0f;

    } else if (_weights != nullptr) {
      weights[i] = weight * _weights->get_weight(i);

    } else {
      weights[i] = weight;
    }
  }

  if (has_flags(F_delta | F_pre_delta)) {
    // Additive blend.

    for (i = 0; i < num_joints; i++) {
      s2 = weights[i];
      if (s2 <= 0.0f) {
        continue;
      }

      if (has_flags(F_pre_delta)) {
        // Underlay delta.

        nassert_raise("Pre-delta blending is not implemented.");

      } else {
        // Overlay delta.
        LQuaternion b_rot;
        AnimEvalData::Joint &a_pose = a._pose[i];
        const AnimEvalData::Joint &b_pose = b._pose[i];
        if (i == 0 && source_delta_anims) {
          // Apply the stupid rotation fix for the root joint of delta animations.
          b_rot = b_pose._rotation * root_delta_fixup;
        } else {
          b_rot = b_pose._rotation;
        }
        a_pose._position += (b_pose._position * s2);
        quaternion_ma_seq(a_pose._rotation, s2, b_rot, a_pose._rotation);
        // Not doing scale or shear.
      }
    }

  } else {
    // Mix blend.
    LQuaternion q3;
    for (i = 0; i < num_joints; i++) {
      s2 = weights[i];
      if (s2 <= 0.0f) {
        continue;
      }

      s1 = 1.0f - s2;

      AnimEvalData::Joint &a_pose = a._pose[i];
      const AnimEvalData::Joint &b_pose = b._pose[i];

      a_pose._position = (a_pose._position * s1) + (b_pose._position * s2);
      a_pose._scale = (a_pose._scale * s1) + (b_pose._scale * s2);
      a_pose._shear = (a_pose._shear * s1) + (b_pose._shear * s2);

      LQuaternion::slerp(b_pose._rotation, a_pose._rotation, s1, q3);
      a_pose._rotation = q3;

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

  IKHelper ik_helper(&context, this);

  if (ik_enable) {
    ik_helper.pre_ik(data);
  }

  AnimEvalData this_data(data, context._num_joints);

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
    this_data._pose[0]._position[0] = 0.0f;
  }
  if (has_flags(F_zero_root_y)) {
    this_data._pose[0]._position[1] = 0.0f;
  }
  if (has_flags(F_zero_root_z)) {
    this_data._pose[0]._position[2] = 0.0f;
  }

  // Now blend the channel onto the output using the requested weight.
  blend(context, data, this_data, data._weight);

  if (ik_enable) {
    ik_helper.apply_ik(data);
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
  }

  manager->read_pointer(scan); // _weights
}
