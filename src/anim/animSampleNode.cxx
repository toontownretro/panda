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
#include "ssemath.h"
#include "ssequaternion.h"
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

  bool frame_blend_flag = context._frame_blend;

  int frame = _control->get_frame();

  AnimBundle *anim = _control->get_anim();
  int num_channels = anim->get_num_joint_channels();
  const JointFrameData *channel_data = anim->get_joint_channel_data().v().data();

  int channel_index = _control->get_channel_index();
  if (channel_index < 0) {
    return;
  }

  LPoint3 ipos;
  LVector3 iscale;
  LQuaternion iquat, next_quat;

  if (!frame_blend_flag) {
    // Hold the current frame until the next one is ready.
    for (int i = 0; i < context._num_joints; i++) {
      JointTransform &xform = context._joints[i];
      CharacterJoint &joint = context._parts[i];
      const JointFrameData &channel = channel_data[
          AnimBundle::get_channel_data_index(num_channels, frame, joint.get_bound(channel_index))];

      xform._position = channel.pos;
      xform._rotation = channel.quat;
      xform._scale = channel.scale;
    }

  } else {
    // Frame blending is enabled.  Need to blend between successive frames.

    sse_blend.start();

    int joint_groups = context._num_joints / 4;
    int strays = context._num_joints % 4;

    PN_stdfloat frac = (PN_stdfloat)_control->get_frac();
    PN_stdfloat e0 = 1.0f - frac;

    fltx4 frac4 = ReplicateX4(frac);
    fltx4 e0_4 = ReplicateX4(e0);

    int next_frame = _control->get_next_frame();

    FourVectors pos, scale, next_pos, next_scale;
    FourQuaternions quat, next_quat_sse;

    for (int i = 0; i < joint_groups; i++) {
      sse_blend_fetch.start();

      int joint_base = i * 4;

      JointTransform &t1 = context._joints[joint_base];
      JointTransform &t2 = context._joints[joint_base + 1];
      JointTransform &t3 = context._joints[joint_base + 2];
      JointTransform &t4 = context._joints[joint_base + 3];

      CharacterJoint &j1 = context._parts[joint_base];
      CharacterJoint &j2 = context._parts[joint_base + 1];
      CharacterJoint &j3 = context._parts[joint_base + 2];
      CharacterJoint &j4 = context._parts[joint_base + 3];

      int j1_bound = j1.get_bound(channel_index);
      int j2_bound = j2.get_bound(channel_index);
      int j3_bound = j3.get_bound(channel_index);
      int j4_bound = j4.get_bound(channel_index);

      const JointFrameData &f1 = channel_data[AnimBundle::get_channel_data_index(num_channels, frame, j1_bound)];
      const JointFrameData &f2 = channel_data[AnimBundle::get_channel_data_index(num_channels, frame, j2_bound)];
      const JointFrameData &f3 = channel_data[AnimBundle::get_channel_data_index(num_channels, frame, j3_bound)];
      const JointFrameData &f4 = channel_data[AnimBundle::get_channel_data_index(num_channels, frame, j4_bound)];

      const JointFrameData &f1_next = channel_data[AnimBundle::get_channel_data_index(num_channels, next_frame, j1_bound)];
      const JointFrameData &f2_next = channel_data[AnimBundle::get_channel_data_index(num_channels, next_frame, j2_bound)];
      const JointFrameData &f3_next = channel_data[AnimBundle::get_channel_data_index(num_channels, next_frame, j3_bound)];
      const JointFrameData &f4_next = channel_data[AnimBundle::get_channel_data_index(num_channels, next_frame, j4_bound)];

      sse_blend_fetch.stop();

      sse_blend_load.start();

      pos.LoadAndSwizzleAligned(f1.pos, f2.pos, f3.pos, f4.pos);
      quat.LoadAndSwizzleAligned(&f1.quat, &f2.quat, &f3.quat, &f4.quat);
      scale.LoadAndSwizzleAligned(f1.scale, f2.scale, f3.scale, f4.scale);
      next_pos.LoadAndSwizzleAligned(f1_next.pos, f2_next.pos, f3_next.pos, f4_next.pos);
      next_quat_sse.LoadAndSwizzleAligned(&f1_next.quat, &f2_next.quat, &f3_next.quat, &f4_next.quat);
      next_scale.LoadAndSwizzleAligned(f1_next.scale, f2_next.scale, f3_next.scale, f4_next.scale);

      sse_blend_load.stop();

      sse_blend_compute.start();

      pos *= e0_4;
      next_pos *= frac4;
      pos += next_pos;

      scale *= e0_4;
      next_scale *= frac4;
      scale += next_scale;

      quat = quat.Blend(next_quat_sse, frac4);

      sse_blend_compute.stop();

      sse_blend_store.start();

      quat.SwizzleAndStoreAligned(&t1._rotation, &t2._rotation, &t3._rotation, &t4._rotation);

      t1._position = pos.Vec(0);
      t1._scale = scale.Vec(0);

      t2._position = pos.Vec(1);
      t2._scale = scale.Vec(1);

      t3._position = pos.Vec(2);
      t3._scale = scale.Vec(2);

      t4._position = pos.Vec(3);
      t4._scale = scale.Vec(3);

      sse_blend_store.stop();
    }

    sse_blend.stop();

    blend_stragglers.start();

    // Do the strays without SSE.

    int stray_start = joint_groups * 4;
    int stray_end = stray_start + strays;

    for (int i = stray_start; i < stray_end; i++) {
      JointTransform &t = context._joints[i];
      CharacterJoint &j = context._parts[i];
      int bound = j.get_bound(channel_index);
      const JointFrameData &c = channel_data[AnimBundle::get_channel_data_index(num_channels, frame, bound)];
      const JointFrameData &c_next = channel_data[AnimBundle::get_channel_data_index(num_channels, next_frame, bound)];

      t._position = c.pos * e0;
      t._scale = c.scale * e0;
      t._position += c_next.pos * frac;
      t._scale += c_next.scale * frac;

      LQuaternion::blend(c.quat, c_next.quat, frac, t._rotation);
    }

    blend_stragglers.stop();
  }
}
