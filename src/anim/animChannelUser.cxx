/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelUser.cxx
 * @author brian
 * @date 2022-12-29
 */

#include "animChannelUser.h"

/**
 *
 */
AnimChannelUser::
AnimChannelUser(const std::string &name, Character *character, bool delta) :
  AnimChannel(name)
{
  int num_joint_groups = max_character_joints / SIMDFloatVector::num_columns;
  for (int i = 0; i < num_joint_groups; ++i) {
    _pose_data._pose[i].pos.fill(0.0f);
    _pose_data._pose[i].shear.fill(0.0f);
    if (delta) {
      _pose_data._pose[i].scale.fill(0.0f);
      _pose_data._pose[i].quat = LQuaternion(0.0f);
    } else {
      _pose_data._pose[i].scale.fill(1.0f);
      _pose_data._pose[i].quat = LQuaternion::ident_quat();
    }
  }
  for (int i = 0; i < num_joint_groups; ++i) {
    _pose_data._sliders[i] = 0.0f;
  }
}

/**
 *
 */
AnimChannelUser::
AnimChannelUser(const AnimChannelUser &copy) :
  AnimChannel(copy)
{
  _pose_data.copy_pose(copy._pose_data, max_character_joints / SIMDFloatVector::num_columns);
}

/**
 *
 */
PT(AnimChannel) AnimChannelUser::
make_copy() const {
  return new AnimChannelUser(*this);
}

/**
 *
 */
PN_stdfloat AnimChannelUser::
get_length(Character *character) const {
  return 0.1f;
}

/**
 *
 */
void AnimChannelUser::
do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data) {
  // Simply copy the user-provided pose into the output.
  this_data.copy_pose(_pose_data, context._num_joint_groups);
}

/**
 *
 */
LVector3 AnimChannelUser::
get_root_motion_vector(Character *character) const {
  return LVector3(0.0f);
}
