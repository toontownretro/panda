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

/**
 *
 */
IKHelper::
IKHelper(const AnimEvalContext *context, const AnimChannel *channel) {
  _context = context;

  if (channel->get_num_ik_rules() == 0 && channel->get_num_ik_locks() == 0) {
    return;
  }

  _joint_net_transforms.resize(context->_num_joints);

  _ik_states.reserve(channel->get_num_ik_locks() + channel->get_num_ik_rules());

  for (int i = 0; i < channel->get_num_ik_locks(); ++i) {
    const AnimChannel::IKLock *lock = channel->get_ik_lock(i);
    IKState state;
    state._type = IKState::T_lock;
    state._chain = lock->_chain;
    _ik_states.push_back(std::move(state));
  }

  for (int i = 0; i < channel->get_num_ik_rules(); ++i) {
    const AnimChannel::IKRule *rule = channel->get_ik_rule(i);
    IKState state;
    state._type = IKState::T_touch;
    state._chain = rule->_chain;
    state._touch = rule->_touch_joint;
    _ik_states.push_back(std::move(state));
  }
}

/**
 * Performs pre-IK computations on the current animation pose.
 */
void IKHelper::
pre_ik(const AnimEvalData &pose) {
  for (size_t i = 0; i < _ik_states.size(); ++i) {
    IKState *state = &_ik_states[i];
    const IKChain *chain = _context->_character->get_ik_chain(state->_chain);
    int joint = chain->get_end_joint();

    if (!CheckBit(_context->_joint_mask, joint)) {
      // Joint not being animated so don't do IK.
      continue;
    }

    // Perform pre-computation based on IK type.
    switch (state->_type) {
    case IKState::T_lock:
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
    case IKState::T_touch:
      {
        // For touches, the target is the delta matrix from the touch joint
        // to the end-effector in the current pose.
        calc_joint_net_transform(joint, pose);
        calc_joint_net_transform(state->_touch, pose);

        LMatrix4 touch_inverse;
        touch_inverse.invert_from(_joint_net_transforms[state->_touch]);

        state->_target = _joint_net_transforms[joint] * touch_inverse;
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
apply_ik(AnimEvalData &data) {
  // Net transforms need to be recomputed from the new pose.
  _joint_net_computed_mask.clear();

  for (size_t i = 0; i < _ik_states.size(); ++i) {
    IKState *state = &_ik_states[i];
    const IKChain *chain = _context->_character->get_ik_chain(state->_chain);
    int joint = chain->get_end_joint();

    if (!CheckBit(_context->_joint_mask, joint)) {
      // Joint not being animated so don't do IK.
      continue;
    }

    switch (state->_type) {
    case IKState::T_lock:
      {
        // Grab chain net transform in current pose.
        calc_joint_net_transform(joint, data);

        // Solve the IK.
        LPoint3 target_end_effector = state->_target.get_row3(3);
        solve_ik(state->_chain, _context->_character, target_end_effector, _joint_net_transforms.data());

        // Maintain original end-effector rotation.
        LVecBase3 scale, shear, pos, hpr;
        decompose_matrix(_joint_net_transforms[joint], scale, shear, hpr, pos);
        _joint_net_transforms[joint] = LMatrix4::scale_shear_mat(scale, shear) * state->_target_rot;
        _joint_net_transforms[joint].set_row(3, pos);

        // Convert back to local space, apply to output pose.
        joint_net_to_local(chain->get_end_joint(), _joint_net_transforms.data(), data, *_context);
        joint_net_to_local(chain->get_middle_joint(), _joint_net_transforms.data(), data, *_context);
        joint_net_to_local(chain->get_top_joint(), _joint_net_transforms.data(), data, *_context);
      }
      break;
    case IKState::T_touch:
      {
        // Grab chain and touch joint net transforms in current pose.
        calc_joint_net_transform(joint, data);
        calc_joint_net_transform(state->_touch, data);

        // Apply target delta to current touch joint matrix to get end-effector
        // goal.
        LMatrix4 end_effector_target_matrix = state->_target * _joint_net_transforms[state->_touch];
        LPoint3 end_effector_target = end_effector_target_matrix.get_row3(3);

        // Solve the IK.
        solve_ik(state->_chain, _context->_character, end_effector_target, _joint_net_transforms.data());

        // Slam the target matrix.
        _joint_net_transforms[joint] = end_effector_target_matrix;

        // Convert back to local space, apply to output pose.
        joint_net_to_local(chain->get_end_joint(), _joint_net_transforms.data(), data, *_context);
        joint_net_to_local(chain->get_middle_joint(), _joint_net_transforms.data(), data, *_context);
        joint_net_to_local(chain->get_top_joint(), _joint_net_transforms.data(), data, *_context);
      }
      break;
    default:
      break;
    }
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

  // Compose a matrix of the current parent-space joint pose.
  const AnimEvalData::Joint &jpose = pose._pose[joint];
  LMatrix4 local = LMatrix4::scale_shear_mat(jpose._scale.get_xyz(), jpose._shear.get_xyz()) * jpose._rotation;
  local.set_row(3, jpose._position.get_xyz());

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
                   AnimEvalData &pose, const AnimEvalContext &context) {
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

  pose._pose[joint]._position = LVecBase4(pos, 1.0f);
  pose._pose[joint]._rotation = quat;
  pose._pose[joint]._scale = LVecBase4(scale, 1.0f);
  pose._pose[joint]._shear = LVecBase4(shear, 1.0f);
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
