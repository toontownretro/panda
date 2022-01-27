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

IMPLEMENT_CLASS(AnimChannel);

NodePath AnimChannel::_render;

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
 * Adds a new IK lock to the channel for the indicated IK chain.
 * Uses IK to move the chain back to where it was before this AnimChannel's
 * pose was applied to the overall pose.
 */
void AnimChannel::
add_ik_lock(int chain, PN_stdfloat pos_weight, PN_stdfloat rot_weight) {
  IKLock lock;
  lock._chain = chain;
  lock._pos_weight = pos_weight;
  lock._rot_weight = rot_weight;
  _ik_locks.push_back(std::move(lock));
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

  // If we have IK locks, remember original end-effector positions.
  LVecBase4 *chain_pos = nullptr;
  LQuaternion *chain_rot = nullptr;
  LVector3 *chain_knee_dir = nullptr;
  LPoint3 *chain_knee_pos = nullptr;
  LMatrix4 *joint_net_transforms = nullptr;
  unsigned char *joint_computed_mask = nullptr;
  if (!_ik_locks.empty()) {
    chain_pos = (LVecBase4 *)alloca(sizeof(LVecBase4) * _ik_locks.size());
    chain_rot = (LQuaternion *)alloca(sizeof(LQuaternion) * _ik_locks.size());
    chain_knee_dir = (LVector3 *)alloca(sizeof(LVector3) * _ik_locks.size());
    chain_knee_pos = (LPoint3 *)alloca(sizeof(LPoint3) * _ik_locks.size());

    // We require the net transforms of the joints in each chain at the
    // current intermediate pose.
    joint_net_transforms = (LMatrix4 *)alloca(sizeof(LMatrix4) * context._num_joints);
    joint_computed_mask = (unsigned char *)alloca(context._num_joints / 8);
    memset(joint_computed_mask, 0, context._num_joints / 8);

    for (size_t i = 0; i < _ik_locks.size(); ++i) {
      const IKChain *chain = context._character->get_ik_chain(_ik_locks[i]._chain);
      int joint = chain->get_end_joint();
      if (!CheckBit(context._joint_mask, joint)) {
        continue;
      }

      r_calc_joint_net_transform(context._character, joint_computed_mask, context, data, joint, joint_net_transforms);

      // Extract net translation and rotation from current joint pose.
      LVecBase3 scale, shear, hpr, pos;
      decompose_matrix(joint_net_transforms[joint], scale, shear, hpr, pos);
      chain_pos[i] = LVecBase4(pos, 1.0f);
      chain_rot[i].set_hpr(hpr);

      if (chain->get_middle_joint_direction().length_squared() > 0.0f) {
        chain_knee_dir[i] = joint_net_transforms[joint].xform_vec(chain->get_middle_joint_direction());
        chain_knee_pos[i] = joint_net_transforms[chain->get_middle_joint()].get_row3(3);

      } else {
        chain_knee_dir[i].set(0.0f, 0.0f, 0.0f);
      }
    }

    memset(joint_computed_mask, 0, context._num_joints / 8);
  }

  // Now blend the channel onto the output using the requested weight.
  blend(context, data, this_data, data._weight);

  // Now solve the locks.

  for (size_t i = 0; i < _ik_locks.size(); ++i) {
    IKLock *lock = &_ik_locks[i];
    const IKChain *chain = context._character->get_ik_chain(lock->_chain);
    int joint = chain->get_end_joint();

    if (!CheckBit(context._joint_mask, joint)) {
      continue;
    }

    if (lock->_smiley0.is_empty()) {
      lock->_smiley0 = NodePath(Loader::get_global_ptr()->load_sync("models/misc/smiley"));
      lock->_smiley0.set_scale(8.0f);
      if (i == 0) {
        lock->_smiley0.set_color_scale(LColor(1, 0, 0, 1));
      } else {
        lock->_smiley0.set_color_scale(LColor(0, 1, 0, 1));
      }
      lock->_smiley0.reparent_to(_render);

      lock->_smiley1 = lock->_smiley0.copy_to(_render);
      if (i == 0) {
        lock->_smiley1.set_color_scale(LColor(0.75, 0, 0, 1));
      } else {
        lock->_smiley1.set_color_scale(LColor(0, 0.75, 0, 1));
      }

      lock->_smiley2 = lock->_smiley0.copy_to(_render);
      if (i == 0) {
        lock->_smiley2.set_color_scale(LColor(0.5, 0, 0, 1));
      } else {
        lock->_smiley2.set_color_scale(LColor(0, 0.5, 0, 1));
      }
    }

    // Make sure we know the net transforms of the chain at the current pose.
    r_calc_joint_net_transform(context._character, joint_computed_mask, context,
                               data, joint, joint_net_transforms);

    LPoint3 p1, p2, p3;
    LQuaternion q2, q3;

    // Extract translation.
    p1 = joint_net_transforms[joint].get_row3(3);

    // Blend in position.
    p3 = p1 * (1.0 - lock->_pos_weight) + chain_pos[i].get_xyz() * lock->_pos_weight;

    // Do exact IK solution.
    if (chain_knee_dir[i].length_squared() > 0.0f) {
      // solve ik
      solve_ik(chain->get_top_joint(), chain->get_middle_joint(), chain->get_end_joint(), p3, chain_knee_pos[i], chain_knee_dir[i], joint_net_transforms);

    } else {
      // solve ik
      solve_ik(lock->_chain, context._character, p3, joint_net_transforms);
    }

    // Apply IK'd position to end-effector, keep original rotation.
    LVecBase3 hpr, pos, scale, shear;
    decompose_matrix(joint_net_transforms[joint], scale, shear, hpr, pos);
    joint_net_transforms[joint] = LMatrix4::scale_shear_mat(scale, shear) * chain_rot[i];
    joint_net_transforms[joint].set_row(3, p3);

    if (!lock->_smiley0.is_empty()) {
      lock->_smiley0.set_mat(joint_net_transforms[chain->get_end_joint()]);
      lock->_smiley1.set_mat(joint_net_transforms[chain->get_middle_joint()]);
      lock->_smiley2.set_mat(joint_net_transforms[chain->get_top_joint()]);
    }

    // Convert back to local space.
    LMatrix4 inv_mat;
    LMatrix4 local;
    LQuaternion rot;
    int parent;

    parent = context._character->get_joint_parent(chain->get_end_joint());
    inv_mat.invert_from(joint_net_transforms[parent]);
    local = joint_net_transforms[chain->get_end_joint()] * inv_mat;
    decompose_matrix(local, scale, shear, hpr, pos);

    rot.set_hpr(hpr);
    LQuaternion::slerp(data._pose[chain->get_end_joint()]._rotation, rot, 1.0f - lock->_rot_weight, q2);
    data._pose[chain->get_end_joint()]._position = LVecBase4(pos, 1.0f);
    data._pose[chain->get_end_joint()]._rotation = rot;
    data._pose[chain->get_end_joint()]._scale = LVecBase4(scale, 1.0f);
    data._pose[chain->get_end_joint()]._shear = LVecBase4(shear, 1.0f);

    parent = context._character->get_joint_parent(chain->get_middle_joint());
    inv_mat.invert_from(joint_net_transforms[parent]);
    local = joint_net_transforms[chain->get_middle_joint()] * inv_mat;
    decompose_matrix(local, scale, shear, hpr, pos);
    rot.set_hpr(hpr);
    data._pose[chain->get_middle_joint()]._position = LVecBase4(pos, 1.0f);
    data._pose[chain->get_middle_joint()]._rotation = rot;
    data._pose[chain->get_middle_joint()]._scale = LVecBase4(scale, 1.0f);
    data._pose[chain->get_middle_joint()]._shear = LVecBase4(shear, 1.0f);

    parent = context._character->get_joint_parent(chain->get_top_joint());
    inv_mat.invert_from(joint_net_transforms[parent]);
    local = joint_net_transforms[chain->get_top_joint()] * inv_mat;
    decompose_matrix(local, scale, shear, hpr, pos);
    rot.set_hpr(hpr);
    data._pose[chain->get_top_joint()]._position = LVecBase4(pos, 1.0f);
    data._pose[chain->get_top_joint()]._rotation = rot;
    data._pose[chain->get_top_joint()]._scale = LVecBase4(scale, 1.0f);
    data._pose[chain->get_top_joint()]._shear = LVecBase4(shear, 1.0f);
  }
}

#define KNEEMAX_EPSILON 0.9998

/**
 *
 */
bool AnimChannel::
solve_ik(int hip, int knee, int foot, LPoint3 &target_foot, LPoint3 &target_knee_pos, LVector3 &target_knee_dir, LMatrix4 *net_transforms) {
  LPoint3 world_foot, world_knee, world_hip;

  world_foot = net_transforms[foot].get_row3(3);
  world_knee = net_transforms[knee].get_row3(3);
  world_hip = net_transforms[hip].get_row3(3);

  LVecBase3 ik_foot, ik_target_knee, ik_knee;

  ik_foot = target_foot - world_hip;
  ik_knee = target_knee_pos - world_hip;

  PN_stdfloat l1 = (world_knee - world_hip).length();
  PN_stdfloat l2 = (world_foot - world_knee).length();

  PN_stdfloat d = (target_foot - world_hip).length() - std::min(l1, l2);
  d = std::max(l1 + l2, d);
  d *= 100;

  ik_target_knee = ik_knee + target_knee_dir * d;

  if (ik_foot.length() > (l1 + l2) * KNEEMAX_EPSILON) {
    ik_foot.normalize();
    ik_foot *= (l1 + l2) * KNEEMAX_EPSILON;
  }

  PN_stdfloat min_dist = std::max(std::abs(l1 - l2) * 1.15f, std::min(l1, l2) * 0.15f);
  if (ik_foot.length() < min_dist) {
    ik_foot = world_foot - world_hip;
    ik_foot.normalize();
    ik_foot *= min_dist;
  }

  IKSolver ik;
  if (ik.solve(l1, l2, ik_foot.get_data(), ik_target_knee.get_data(), (float *)ik_knee.get_data())) {

    align_ik_matrix(net_transforms[hip], ik_knee);
    align_ik_matrix(net_transforms[knee], ik_foot - ik_knee);

    net_transforms[knee].set_row(3, ik_knee + world_hip);
    net_transforms[foot].set_row(3, ik_foot + world_hip);

    return true;

  } else {
    return false;
  }

}

/**
 *
 */
bool AnimChannel::
solve_ik(int chain, Character *character, LPoint3 &target_foot, LMatrix4 *net_transforms) {
  const IKChain *ikchain = character->get_ik_chain(chain);

  if (ikchain->get_middle_joint_direction().length_squared() > 0.0f) {
    LVector3 target_knee_dir;
    LPoint3 target_knee_pos;
    LVector3 tmp = ikchain->get_middle_joint_direction();
    target_knee_dir = net_transforms[ikchain->get_top_joint()].xform_vec(tmp);
    target_knee_pos = net_transforms[ikchain->get_middle_joint()].get_row3(3);

    return solve_ik(ikchain->get_top_joint(), ikchain->get_middle_joint(), ikchain->get_end_joint(),
                    target_foot, target_knee_pos, target_knee_dir, net_transforms);

  } else {
    return solve_ik(ikchain->get_top_joint(), ikchain->get_middle_joint(), ikchain->get_end_joint(),
                    target_foot, net_transforms);
  }
}

/**
 *
 */
bool AnimChannel::
solve_ik(int hip, int knee, int foot, LPoint3 &target_foot, LMatrix4 *net_transforms) {
  LPoint3 world_foot, world_knee, world_hip;

  world_foot = net_transforms[foot].get_row3(3);
  world_knee = net_transforms[knee].get_row3(3);
  world_hip = net_transforms[hip].get_row3(3);

  LVecBase3 ik_foot, ik_knee;

  ik_foot = target_foot - world_hip;
  ik_knee = world_knee - world_hip;

  PN_stdfloat l1 = (world_knee - world_hip).length();
  PN_stdfloat l2 = (world_foot - world_knee).length();
  PN_stdfloat l3 = (world_foot - world_hip).length();

  if (l3 > (l1 + l2) * KNEEMAX_EPSILON) {
    return false;
  }

  LVecBase3 ik_half = (world_foot - world_hip) * (l1 / l3);

  LVector3 ik_knee_dir = ik_knee - ik_half;
  ik_knee_dir.normalize();

  return solve_ik(hip, knee, foot, target_foot, world_knee, ik_knee_dir, net_transforms);
}

/**
 *
 */
void AnimChannel::
align_ik_matrix(LMatrix4 &mat, const LVecBase3 &align_to) {
  LVecBase3 tmp1, tmp2, tmp3;

  tmp1 = align_to;
  tmp1.normalize();
  mat.set_row(0, tmp1);

  tmp3 = mat.get_row3(2);
  tmp2 = tmp3.cross(tmp1);
  tmp2.normalize();
  mat.set_row(1, tmp2);

  tmp3 = tmp1.cross(tmp2);
  mat.set_row(2, tmp3);
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

  me.add_uint8(_ik_locks.size());
  for (size_t i = 0; i < _ik_locks.size(); ++i) {
    me.add_int8(_ik_locks[i]._chain);
    me.add_stdfloat(_ik_locks[i]._pos_weight);
    me.add_stdfloat(_ik_locks[i]._rot_weight);
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

  _ik_locks.resize(scan.get_uint8());
  for (size_t i = 0; i < _ik_locks.size(); ++i) {
    _ik_locks[i]._chain = scan.get_int8();
    _ik_locks[i]._pos_weight = scan.get_stdfloat();
    _ik_locks[i]._rot_weight = scan.get_stdfloat();
  }

  manager->read_pointer(scan); // _weights
}

/**
 *
 */
void AnimChannel::
r_calc_joint_net_transform(const Character *character, unsigned char *joint_computed_mask,
                           const AnimEvalContext &context, AnimEvalData &pose, int joint,
                           LMatrix4 *net_transforms) {
  if (CheckBit(joint_computed_mask, joint)) {
    return;
  }

  // Build local joint matrix.
  const AnimEvalData::Joint &jpose = pose._pose[joint];
  LMatrix4 matrix = LMatrix4::scale_shear_mat(jpose._scale.get_xyz(), jpose._shear.get_xyz()) * jpose._rotation;
  matrix.set_row(3, jpose._position.get_xyz());

  int parent = character->get_joint_parent(joint);
  if (parent == -1) {
    net_transforms[joint] = matrix * character->get_root_xform();

  } else {
    // Recurse up the hierarchy.
    r_calc_joint_net_transform(character, joint_computed_mask, context, pose, parent, net_transforms);
    net_transforms[joint] = matrix * net_transforms[parent];
  }

  SetBit(joint_computed_mask, joint);
}

/**
 *
 */
void AnimChannel::
set_render(const NodePath &render) {
  _render = render;
}
