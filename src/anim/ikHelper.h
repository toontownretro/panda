/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikHelper.h
 * @author brian
 * @date 2022-01-29
 */

#ifndef IKHELPER_H
#define IKHELPER_H

#include "pandabase.h"
#include "bitArray.h"
#include "luse.h"
#include "animChannel.h"

class AnimEvalContext;
class AnimEvalData;
class Character;
class IKChain;

/**
 * Contains the state of an IK operation for a particular IK chain.
 */
class EXPCL_PANDA_ANIM IKState {
public:
  IKState() = default;
  IKState(const IKState &copy) = default;

  const IKChain *_chain;
  const AnimChannel::IKEvent *_event;

  // Target end-effector net transform.
  LMatrix4 _target;
  LQuaternion _target_rot;

  PN_stdfloat _blend_val;
};

/**
 *
 */
class EXPCL_PANDA_ANIM IKHelper {
public:
  IKHelper(const AnimEvalContext *context, const AnimChannel *channel);

  void pre_ik(const AnimEvalData &pose);
  void apply_ik(AnimEvalData &pose);

  void calc_joint_net_transform(int joint, const AnimEvalData &pose);
  bool solve_ik(int hip, int knee, int foot, LPoint3 &target_foot, LPoint3 &target_knee_pos, LVector3 &target_knee_dir, LMatrix4 *net_transforms);
  bool solve_ik(int chain, Character *character, LPoint3 &target_foot, LMatrix4 *net_transforms);
  bool solve_ik(int hip, int knee, int foot, LPoint3 &target_foot, LMatrix4 *net_transforms);
  void align_ik_matrix(LMatrix4 &mat, const LVecBase3 &align_to);
  void joint_net_to_local(int joint, LMatrix4 *net_transforms, AnimEvalData &pose, const AnimEvalContext &context, PN_stdfloat weight);

public:
  const AnimEvalContext *_context;

  // Holds a bit for each joint that indicates whether or not a net transform
  // was computed for it in the _joint_net_transforms vector.
  BitArray _joint_net_computed_mask;
  // Net transform matrix for each character joint.
  pvector<LMatrix4> _joint_net_transforms;

  pvector<IKState> _ik_states;
};

#include "ikHelper.I"

#endif // IKHELPER_H
