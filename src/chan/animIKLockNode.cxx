/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animIKLockNode.cxx
 * @author lachbr
 * @date 2021-02-21
 */

#include "animIKLockNode.h"
#include "ikSolver.h"
#include "movingPartMatrix.h"

/**
 *
 */
void AnimIKLockNode::
evaluate(AnimGraphEvalContext &context) {
  nassertv(_pose != nullptr);

  _pose->evaluate(context);

  if (_ik_chains.size() == 0) {
    return;
  }

  // Now solve each chain.
  for (size_t i = 0; i < _ik_chains.size(); i++) {
    IKChain *chain = _ik_chains[i];

    int foot_index = -1;
    int knee_index = -1;
    int hip_index = -1;

    // FIXME: This is terrible.  Need to cache the joint indices.
    for (int j = 0; j < (int)context._joints.size(); j++) {
      MovingPartMatrix *part = context._parts[j];
      if (part == chain->get_foot()) {
        foot_index = j;
      } else if (part == chain->get_knee()) {
        knee_index = j;
      } else if (part == chain->get_hip()) {
        hip_index = j;
      }

      if (foot_index != -1 && knee_index != -1 && hip_index != -1) {
        break;
      }
    }

    nassertv(foot_index != -1 && knee_index != -1 && hip_index != -1);

    JointTransform &hip = context._joints[hip_index];
    JointTransform &knee = context._joints[knee_index];
    JointTransform &foot = context._joints[foot_index];

    //IKSolver::solve_ik()
  }

}
