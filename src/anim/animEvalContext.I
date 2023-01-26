/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animEvalContext.I
 * @author brian
 * @date 2021-08-04
 */

/**
 *
 */
INLINE AnimEvalData::
AnimEvalData() :
  _weight(1.0f),
  _net_weight(1.0f),
  _cycle(0.0f)
{
}

/**
 *
 */
INLINE AnimEvalData::
AnimEvalData(const AnimEvalData &copy, int num_joint_groups) {
  std::copy(copy._pose, copy._pose + num_joint_groups, _pose);
  // TODO: only copy used sliders.
  std::copy(copy._sliders, copy._sliders + (max_character_joints / SIMDFloatVector::num_columns), _sliders);

  _weight = copy._weight;
  _net_weight = copy._net_weight;
  _cycle = copy._cycle;
}

/**
 *
 */
INLINE AnimEvalData::
AnimEvalData(AnimEvalData &&other, int num_joint_groups) {
  std::move(other._pose, other._pose + num_joint_groups, _pose);
  // TODO: only move used sliders.
  std::move(other._sliders, other._sliders + (max_character_joints / SIMDFloatVector::num_columns), _sliders);

  _weight = std::move(other._weight);
  _net_weight = std::move(other._net_weight);
  _cycle = std::move(other._cycle);
}

/**
 * Copies the joint poses from the indicated AnimEvalData into this one.
 */
INLINE void AnimEvalData::
copy_pose(const AnimEvalData &copy, int num_joint_groups) {
  std::copy(copy._pose, copy._pose + num_joint_groups, _pose);
  // TODO: only copy used sliders.
  std::copy(copy._sliders, copy._sliders + (max_character_joints / SIMDFloatVector::num_columns), _sliders);
}

/**
 * Moves the joint poses from the indicated AnimEvalData into this one.  Do not
 * try to access the joint poses on the other AnimEvalData after calling this.
 */
INLINE void AnimEvalData::
steal_pose(AnimEvalData &other, int num_joint_groups) {
  std::move(other._pose, other._pose + num_joint_groups, _pose);
  // TODO: only move used sliders.
  std::move(other._sliders, other._sliders + (max_character_joints / SIMDFloatVector::num_columns), _sliders);
}
