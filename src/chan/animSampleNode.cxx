/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSampleNode.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animSampleNode.h"
#include "movingPartMatrix.h"
#include "animChannel.h"

#define DELEGATE_TO_CONTROL(func) \
  if (_control != nullptr) { \
    _control->func; \
  }

TypeHandle AnimSampleNode::_type_handle;

/**
 *
 */
AnimSampleNode::
AnimSampleNode(const std::string &name) :
  AnimGraphNode(name),
  _control(nullptr)
{
}

/**
 * Runs the entire animation from beginning to end and stops.
 */
void AnimSampleNode::
play() {
  DELEGATE_TO_CONTROL(play());
}

/**
 * Runs the animation from the frame "from" to and including the frame "to",
 * at which point the animation is stopped.  Both "from" and "to" frame
 * numbers may be outside the range (0, get_num_frames()) and the animation
 * will follow the range correctly, reporting numbers modulo get_num_frames().
 * For instance, play(0, get_num_frames() * 2) will play the animation twice
 * and then stop.
 */
void AnimSampleNode::
play(double from, double to) {
  DELEGATE_TO_CONTROL(play(from, to));
}

/**
 * Starts the entire animation looping.  If restart is true, the animation is
 * restarted from the beginning; otherwise, it continues from the current
 * frame.
 */
void AnimSampleNode::
loop(bool restart) {
  DELEGATE_TO_CONTROL(loop(restart));
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * indefinitely.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimSampleNode::
loop(bool restart, double from, double to) {
  DELEGATE_TO_CONTROL(loop(restart, from, to));
}

/**
 * Starts the entire animation bouncing back and forth between its first frame
 * and last frame.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimSampleNode::
pingpong(bool restart) {
  DELEGATE_TO_CONTROL(pingpong(restart));
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * and then back in the opposite direction, indefinitely.
 */
void AnimSampleNode::
pingpong(bool restart, double from, double to) {
  DELEGATE_TO_CONTROL(pingpong(restart, from, to));
}

/**
 * Stops a currently playing or looping animation right where it is.  The
 * animation remains posed at the current frame.
 */
void AnimSampleNode::
stop() {
  DELEGATE_TO_CONTROL(stop());
}

/**
 * Sets the animation to the indicated frame and holds it there.
 */
void AnimSampleNode::
pose(double frame) {
  DELEGATE_TO_CONTROL(pose(frame));
}

/**
 * Changes the rate at which the animation plays.  1.0 is the normal speed,
 * 2.0 is twice normal speed, and 0.5 is half normal speed.  0.0 is legal to
 * pause the animation, and a negative value will play the animation
 * backwards.
 */
void AnimSampleNode::
set_play_rate(double play_rate) {
  DELEGATE_TO_CONTROL(set_play_rate(play_rate));
}

/**
 *
 */
void AnimSampleNode::
evaluate(AnimGraphEvalContext &context) {
  nassertv(_control != nullptr);

  bool frame_blend_flag = context._frame_blend;

  int frame = _control->get_frame();

  LPoint3 ipos;
  LVector3 iscale, ishear;
  LQuaternion iquat;

  for (size_t i = 0; i < context._joints.size(); i++) {
    JointTransform &joint = context._joints[i];
    MovingPartMatrix *part = context._parts[i];

    MovingPartMatrix::ChannelType *channel = nullptr;
    int channel_index = _control->get_channel_index();
    if (channel_index >= 0 && channel_index < part->get_max_bound()) {
      channel = DCAST(MovingPartMatrix::ChannelType, part->get_bound(channel_index));
    }

    if (channel == nullptr) {
      continue;
    }

    channel->get_pos(frame, ipos);
    channel->get_quat(frame, iquat);
    channel->get_scale(frame, iscale);
    channel->get_shear(frame, ishear);

    if (!frame_blend_flag) {
      // Hold the current frame until the next one is ready.
      joint._position = ipos;
      joint._rotation = iquat;
      joint._scale = iscale;
      joint._shear = ishear;

    } else {
      // Frame blending is enabled.  Need to blend between successive frames.

      PN_stdfloat frac = (PN_stdfloat)_control->get_frac();
      PN_stdfloat e0 = 1.0f - frac;

      joint._position = ipos * e0;
      joint._scale = iscale * e0;
      joint._shear = ishear * e0;

      LQuaternion next_quat;
      int next_frame = _control->get_next_frame();
      channel->get_pos(next_frame, ipos);
      channel->get_quat(next_frame, next_quat);
      channel->get_scale(next_frame, iscale);
      channel->get_shear(next_frame, ishear);

      PN_stdfloat e1 = frac;

      joint._position += ipos * e1;
      joint._scale += iscale * e1;
      joint._shear += ishear * e1;

      LQuaternion::blend(iquat, next_quat, frac, joint._rotation);
    }
  }
}
