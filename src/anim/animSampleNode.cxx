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
#include "characterJoint.h"
#include "animBundle.h"
#include "pStatCollector.h"

static PStatCollector sse_blend("*:Animation:Joints:FrameBlendSSE");
static PStatCollector sse_blend_fetch("*:Animation:Joints:FrameBlendSSE:Fetch");
static PStatCollector sse_blend_load("*:Animation:Joints:FrameBlendSSE:Load");
static PStatCollector sse_blend_compute("*:Animation:Joints:FrameBlendSSE:Blend");
static PStatCollector sse_blend_store("*:Animation:Joints:FrameBlendSSE:Store");
static PStatCollector blend_stragglers("*:Animation:Joints:FrameBlendStragglers");

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

  int frame = _control->get_frame();
  int next_frame = _control->get_next_frame();

  AnimBundle *anim = _control->get_anim();

  int channel_index = _control->get_channel_index();
  if (channel_index < 0) {
    return;
  }

  if (!context._frame_blend || frame == next_frame) {
    // Hold the current frame until the next one is ready.
    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &xform = context._joints[i];
      CharacterJoint &joint = context._parts[i];
      int bound = joint.get_bound(channel_index);
      if (bound == -1) {
        continue;
      }
      const JointFrame &jframe = anim->get_joint_frame(bound, frame);

      xform._rotation = jframe.quat;
      xform._position = jframe.pos;
      xform._scale = jframe.scale;
    }

  } else {
    // Frame blending is enabled.  Need to blend between successive frames.

    PN_stdfloat frac = (PN_stdfloat)_control->get_frac();
    PN_stdfloat e0 = 1.0f - frac;

    blend_stragglers.start();

    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &t = context._joints[i];
      CharacterJoint &j = context._parts[i];
      int bound = j.get_bound(channel_index);
      if (bound == -1) {
        continue;
      }

      const JointEntry &je = anim->get_joint_entry(bound);
      const JointFrame &jf = anim->get_joint_frame(je, frame);
      const JointFrame &jf_next = anim->get_joint_frame(je, next_frame);

      t._position = (jf.pos * e0) + (jf_next.pos * frac);
      t._scale = (jf.scale * e0) + (jf_next.scale * frac);
      LQuaternion::blend(jf.quat, jf_next.quat, frac, t._rotation);
    }

    blend_stragglers.stop();
  }
}
