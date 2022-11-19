/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikHelper.cxx
 * @author brian
 * @date 2022-01-29
 */

#include "ikHelper.h"
#include "animEvalContext.h"
#include "character.h"
#include "ikChain.h"
#include "ikSolver.h"
#include "mathutil_misc.h"

#define IK_WEIGHT_EPSILON 0.001f

/**
 *
 */
IKHelper::
IKHelper(const AnimEvalContext *context, bool local) {
  _context = context;
  _character = context->_character;
  _local = local;
}

/**
 *
 */
void IKHelper::
add_channel_events(const AnimChannel *channel, const AnimEvalData &pose) {
  if (channel->get_num_ik_events() == 0) {
    return;
  }

  if (_joint_net_transforms.empty()) {
    _joint_net_transforms.resize(_context->_num_joints);
  }

  if (_ik_states.empty()) {
    _ik_states.resize(_character->get_num_ik_chains());
  }

  // Net transforms need to be recomputed from the new pose.
  _joint_net_computed_mask.clear();

  for (int i = 0; i < channel->get_num_ik_events(); ++i) {
    const AnimChannel::IKEvent *event = channel->get_ik_event(i);

    if (_local && event->_type != AnimChannel::IKEvent::T_lock) {
      // Only doing local IK events.
      continue;
    } else if (!_local && event->_type == AnimChannel::IKEvent::T_lock) {
      // Only doing global IK events.
      continue;
    }

    const IKChain *chain = _character->get_ik_chain(event->_chain);
    int joint = chain->get_end_joint();
    if (!CheckBit(_context->_joint_mask, joint)) {
      continue;
    }

    // Compute blend weight.
    PN_stdfloat start, peak, tail, end;
    start = event->_start;
    peak = event->_peak;
    tail = event->_tail;
    end = event->_end;

    PN_stdfloat blend_val = 1.0f;

    if (start != end) {
      PN_stdfloat index;

      if (event->_pose_parameter == -1) {
        index = pose._cycle;

      } else {
        // Drive blend by pose parameter value.
        const PoseParameter &pp = _context->_character->get_pose_parameter(event->_pose_parameter);
        index = pp.get_value();
      }

      if (index < start || index >= end) {
        // Not in range.
        blend_val = 0.0f;

      } else {
        PN_stdfloat scale = 1.0f;
        if (index < peak && start != peak) {
          // On the way up.
          scale = (index - start) / (peak - start);

        } else if (index > tail && end != tail) {
          // On the way down.
          scale = (end - index) / (end - tail);
        }

        if (event->_spline) {
          // Spline blend.
          scale = simple_spline(scale);
        }

        blend_val = scale;
      }
    }

    //blend_val *= pose._net_weight;

    if (blend_val <= IK_WEIGHT_EPSILON) {
      // Negligible weight.
      continue;
    }

    assert(event->_chain >= 0 && event->_chain < (int)_ik_states.size());
    pvector<IKState> &chain_states = _ik_states[event->_chain];
    if (blend_val >= 0.999f) {
      chain_states.clear();
      if (event->_type == AnimChannel::IKEvent::T_release) {
        continue;
      }
    }
    chain_states.push_back(IKState());
    IKState *state = &chain_states[chain_states.size() - 1];

    state->_event = event;
    state->_chain = chain;
    state->_blend_val = blend_val;

    // Perform pre-computation based on IK type.
    switch (event->_type) {
    case AnimChannel::IKEvent::T_lock:
      {
        // For IK locks, we need to store off the current net transform
        // of the end-effector and use that as the target transform during IK
        // application.
        calc_joint_net_transform(joint, pose);
        state->_target = _joint_net_transforms[joint];
        LVecBase3 scale, shear, hpr, pos;
        decompose_matrix(state->_target, scale, shear, hpr, pos);
        state->_target_rot.set_hpr(hpr);
      }
      break;
    case AnimChannel::IKEvent::T_touch:
      {
        // Store the IK target for touches as the offset from the touch joint to
        // the end effector.
        if (event->_touch_offsets.empty()) {
          // If we don't have touch offsets from a reference animation, use the offset from
          // the touch joint to the end effector in the current pose (before the current channel
          // is applied).
          calc_joint_net_transform(joint, pose);
          calc_joint_net_transform(event->_touch_joint, pose);

          LMatrix4 touch_inverse;
          touch_inverse.invert_from(_joint_net_transforms[event->_touch_joint]);

          state->_target = _joint_net_transforms[joint] * touch_inverse;
        } else {
          // We have per-frame touch offsets from a reference animation.
          float fframe = pose._cycle * event->_touch_offsets.size();
          int frame = (int)(fframe);
          int next_frame = frame + 1;
          if (_context->_play_mode == AnimLayer::PM_loop) {
            next_frame = cmod(next_frame, (int)event->_touch_offsets.size());
          } else {
            next_frame = std::min(next_frame, (int)event->_touch_offsets.size() - 1);
          }
          float frac = fframe - frame;

          LQuaternion touch_rot0, touch_rot1;
          LPoint3 touch_pos0, touch_pos1;
          touch_rot0.set_hpr(event->_touch_offsets[frame]._hpr);
          touch_rot1.set_hpr(event->_touch_offsets[next_frame]._hpr);
          touch_pos0 = event->_touch_offsets[frame]._pos;
          touch_pos1 = event->_touch_offsets[next_frame]._pos;

          LQuaternion touch_rot;
          LQuaternion::blend(touch_rot0, touch_rot1, frac, touch_rot);
          LPoint3 touch_pos = touch_pos0 * (1.0f - frac) + touch_pos1 * frac;

          // The touch offset is relative to the touch joint.
          LMatrix4 touch_mat = LMatrix4::translate_mat(touch_pos) * touch_rot;
          state->_target = touch_mat;
        }

      }
      break;
    case AnimChannel::IKEvent::T_target:
      {
        // Given that the user-specified target matrix is in world-space,
        // move it into character space.
        NodePath character_np = NodePath::any_path((PandaNode *)_context->_character->_active_owner);
        LMatrix4 character_net_inverse;
        character_net_inverse.invert_from(character_np.get_mat(NodePath()));
        state->_target = _context->_character->get_ik_target(event->_touch_joint)->_matrix * character_net_inverse;
        LVecBase3 scale, shear, pos, hpr;
        decompose_matrix(state->_target, scale, shear, hpr, pos);
        state->_target_rot.set_hpr(hpr);
      }
      break;
    default:
      break;
    }
  }
}

/**
 * Solves IK from data collected in pre_ik() and applies the new joint poses
 * to data.
 */
void IKHelper::
apply_ik(AnimEvalData &data, PN_stdfloat weight) {
  if (_ik_states.empty()) {
    return;
  }

  // Net transforms need to be recomputed from the new pose.
  _joint_net_computed_mask.clear();

  float chain_weight[32];
  memset(chain_weight, 0, sizeof(chain_weight));
  // Targets
  LPoint3 chain_pos[32];
  memset(chain_pos, 0, sizeof(chain_pos));
  LQuaternion chain_rot[32];
  memset(chain_rot, 0, sizeof(chain_rot));

  for (size_t i = 0; i < _ik_states.size(); ++i) {
    for (size_t j = 0; j < _ik_states[i].size(); ++j) {
      const IKState *state = &_ik_states[i][j];
      const AnimChannel::IKEvent *event = state->_event;
      if (event == nullptr) {
        continue;
      }
      int chain_index = event->_chain;

      PN_stdfloat state_weight = state->_blend_val * weight;

      switch (event->_type) {
      case AnimChannel::IKEvent::T_lock:
        {
          LPoint3 target_pos = state->_target.get_row3(3);
          LQuaternion target_rot;
          target_rot.set_from_matrix(state->_target);

          chain_weight[chain_index] = chain_weight[chain_index] * (1.0f - state_weight) + state_weight;
          chain_pos[chain_index] = chain_pos[chain_index] * (1.0f - state_weight) + target_pos * state_weight;
          LQuaternion::slerp(chain_rot[chain_index], target_rot, state_weight, chain_rot[chain_index]);
        }
        break;
      case AnimChannel::IKEvent::T_touch:
        {
          calc_joint_net_transform(event->_touch_joint, data);
          // Apply target delta to current touch joint matrix to get end-effector
          // goal.
          LMatrix4 end_effector_target_matrix = state->_target * _joint_net_transforms[event->_touch_joint];
          LPoint3 target_pos = end_effector_target_matrix.get_row3(3);
          LQuaternion target_rot;
          target_rot.set_from_matrix(end_effector_target_matrix);

          chain_weight[chain_index] = chain_weight[chain_index] * (1.0f - state_weight) + state_weight;
          chain_pos[chain_index] = chain_pos[chain_index] * (1.0f - state_weight) + target_pos * state_weight;
          LQuaternion::slerp(chain_rot[chain_index], target_rot, state_weight, chain_rot[chain_index]);
        }
        break;
      case AnimChannel::IKEvent::T_release:
        {
          calc_joint_net_transform(state->_chain->get_end_joint(), data);
          LPoint3 target_pos = _joint_net_transforms[state->_chain->get_end_joint()].get_row3(3);
          LQuaternion target_rot;
          target_rot.set_from_matrix(_joint_net_transforms[state->_chain->get_end_joint()]);

          chain_pos[chain_index] = chain_pos[chain_index] * (1.0f - state_weight) + target_pos * state_weight;
          LQuaternion::slerp(chain_rot[chain_index], target_rot, state_weight, chain_rot[chain_index]);
        }
      default:
        break;
      }
    }
  }

  for (int i = 0; i < _character->get_num_ik_chains(); ++i) {
    if (chain_weight[i] <= 0.0f) {
      continue;
    }

    const IKChain *chain = _character->get_ik_chain(i);

    calc_joint_net_transform(chain->get_end_joint(), data);
    if (!solve_ik(i, _character, chain_pos[i], _joint_net_transforms.data())) {
      continue;
    }

    int end_joint = chain->get_end_joint();

    // Slam target orientation.
    LPoint3 pos = _joint_net_transforms[end_joint].get_row3(3);
    _joint_net_transforms[end_joint] = LMatrix4::translate_mat(pos) * chain_rot[i];

    // Convert back to local space, apply to output pose.
    joint_net_to_local(chain->get_end_joint(), _joint_net_transforms.data(), data, *_context, chain_weight[i]);
    joint_net_to_local(chain->get_middle_joint(), _joint_net_transforms.data(), data, *_context, chain_weight[i]);
    joint_net_to_local(chain->get_top_joint(), _joint_net_transforms.data(), data, *_context, chain_weight[i]);
  }
}

/**
 *
 */
void IKHelper::
calc_joint_net_transform(int joint, const AnimEvalData &pose) {
  if (_joint_net_computed_mask.get_bit(joint)) {
    // We already computed this joint and above.
    return;
  }

  int group = joint / SIMDFloatVector::num_columns;
  int sub = joint % SIMDFloatVector::num_columns;

  // Compose a matrix of the current parent-space joint pose.
  LMatrix4 local = LMatrix4::scale_shear_mat(pose._pose[group].scale.get_lvec(sub),
    pose._pose[group].shear.get_lvec(sub)) * pose._pose[group].quat.get_lquat(sub);
  local.set_row(3, pose._pose[group].pos.get_lvec(sub));

  // Transform local matrix by the parent's net matrix.
  int parent = _context->_character->get_joint_parent(joint);
  if (parent == -1) {
    _joint_net_transforms[joint] = local * _context->_character->get_root_xform();

  } else {
    // Recurse up the hierarchy.
    calc_joint_net_transform(parent, pose);
    _joint_net_transforms[joint] = local * _joint_net_transforms[parent];
  }

  _joint_net_computed_mask.set_bit(joint);
}

/**
 * Transforms the indicated joint's net transform into parent-space and
 * applies it to the given pose data.
 */
void IKHelper::
joint_net_to_local(int joint, LMatrix4 *net_transforms,
                   AnimEvalData &data, const AnimEvalContext &context,
                   PN_stdfloat weight) {
  int parent = context._character->get_joint_parent(joint);
  LMatrix4 parent_net_inverse;
  if (parent == -1) {
    parent_net_inverse.invert_from(context._character->get_root_xform());

  } else {
    parent_net_inverse.invert_from(net_transforms[parent]);
  }

  LMatrix4 local = net_transforms[joint] * parent_net_inverse;

  LVecBase3 scale, shear, hpr, pos;
  decompose_matrix(local, scale, shear, hpr, pos);

  LQuaternion quat;
  quat.set_hpr(hpr);

  PN_stdfloat e0 = 1.0f - weight;

  // Blend between IK'd local pose and existing pose.
  int group = joint / SIMDFloatVector::num_columns;
  int sub = joint % SIMDFloatVector::num_columns;

  LVecBase3f dpos, dscale, dshear;
  LQuaternionf dquat;
  data._pose[group].pos.get_lvec(sub, dpos);
  data._pose[group].scale.get_lvec(sub, dscale);
  data._pose[group].shear.get_lvec(sub, dshear);
  data._pose[group].quat.get_lquat(sub, dquat);

  dpos *= e0;
  dpos += pos * weight;
  dscale *= e0;
  dscale += scale * weight;
  dshear *= e0;
  dshear += shear * weight;
  LQuaternion q2;
  LQuaternion::slerp(dquat, quat, weight, q2);
  dquat = q2;

  data._pose[group].pos.set_lvec(sub, dpos);
  data._pose[group].scale.set_lvec(sub, dscale);
  data._pose[group].shear.set_lvec(sub, dshear);
  data._pose[group].quat.set_lquat(sub, dquat);
}

#define KNEEMAX_EPSILON 0.9998

/**
 * Solves a 2-joint IK with the given end-effector target position and current
 * middle joint position/orientation.
 */
bool IKHelper::
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
 * Solves a 2-joint IK for the indicated IK chain and given end-effector
 * target position.
 */
bool IKHelper::
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
 * Solves a 2-joint IK with a target end-effector position but no preferred
 * middle joint direction/position.
 */
bool IKHelper::
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
void IKHelper::
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
