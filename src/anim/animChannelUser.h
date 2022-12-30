/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelUser.h
 * @author brian
 * @date 2022-12-29
 */

#ifndef ANIMCHANNELUSER_H
#define ANIMCHANNELUSER_H

#include "pandabase.h"
#include "animChannel.h"

/**
 * AnimChannel containing a user-provided pose for each joint and slider
 * of the character.  Allows user to compute animation procedurally and
 * use it as an animation channel in the blend tree.
 */
class EXPCL_PANDA_ANIM AnimChannelUser : public AnimChannel {
PUBLISHED:
  AnimChannelUser(const std::string &name, Character *character, bool delta);
  AnimChannelUser(const AnimChannelUser &copy);

  INLINE void set_joint_position(int i, const LPoint3 &pos);
  INLINE LPoint3 get_joint_position(int i) const;

  INLINE void set_joint_scale(int i, const LVecBase3 &scale);
  INLINE LVecBase3 get_joint_scale(int i) const;

  INLINE void set_joint_shear(int i, const LVecBase3 &shear);
  INLINE LVecBase3 get_joint_shear(int i) const;

  INLINE void set_joint_quat(int i, const LQuaternion &quat);
  INLINE LQuaternion get_joint_quat(int i) const;

  INLINE void set_joint_hpr(int i, const LVecBase3 &hpr);
  INLINE LVecBase3 get_joint_hpr(int i) const;

  INLINE void set_slider(int i, PN_stdfloat value);
  INLINE PN_stdfloat get_slider(int i) const;

  // AnimChannel interface.
  virtual PT(AnimChannel) make_copy() const override;
  virtual PN_stdfloat get_length(Character *character) const override;
  virtual void do_calc_pose(const AnimEvalContext &context, AnimEvalData &this_data) override;
  virtual LVector3 get_root_motion_vector(Character *character) const override;

private:
  AnimEvalData _pose_data;
};

#include "animChannelUser.I"

#endif // ANIMCHANNELUSER_H
