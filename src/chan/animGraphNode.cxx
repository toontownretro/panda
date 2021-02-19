/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animGraphNode.cxx
 * @author lachbr
 * @date 2021-02-18
 */

#include "animGraphNode.h"

TypeHandle AnimGraphNode::_type_handle;

#define FOREACH_INPUT(func) \
  Inputs::const_iterator ii; \
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) { \
    (*ii)->func; \
  }

/**
 *
 */
AnimGraphNode::
AnimGraphNode(const std::string &name) :
  Namable(name)
{
}

/**
 * Returns the maximum allowed number of input nodes to this node.  -1 is
 * infinite.  Nodes with no inputs are leaf nodes, such as AnimSampleNodes.
 */
int AnimGraphNode::
get_max_inputs() const {
  return -1;
}

/**
 * Runs the entire animation from beginning to end and stops.
 */
void AnimGraphNode::
play() {
  FOREACH_INPUT(play());
}

/**
 * Runs the animation from the frame "from" to and including the frame "to",
 * at which point the animation is stopped.  Both "from" and "to" frame
 * numbers may be outside the range (0, get_num_frames()) and the animation
 * will follow the range correctly, reporting numbers modulo get_num_frames().
 * For instance, play(0, get_num_frames() * 2) will play the animation twice
 * and then stop.
 */
void AnimGraphNode::
play(double from, double to) {
  FOREACH_INPUT(play(from, to));
}

/**
 * Starts the entire animation looping.  If restart is true, the animation is
 * restarted from the beginning; otherwise, it continues from the current
 * frame.
 */
void AnimGraphNode::
loop(bool restart) {
  FOREACH_INPUT(loop(restart));
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * indefinitely.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimGraphNode::
loop(bool restart, double from, double to) {
  FOREACH_INPUT(loop(restart, from, to));
}

/**
 * Starts the entire animation bouncing back and forth between its first frame
 * and last frame.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimGraphNode::
pingpong(bool restart) {
  FOREACH_INPUT(pingpong(restart));
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * and then back in the opposite direction, indefinitely.
 */
void AnimGraphNode::
pingpong(bool restart, double from, double to) {
  FOREACH_INPUT(pingpong(restart, from, to));
}

/**
 * Stops a currently playing or looping animation right where it is.  The
 * animation remains posed at the current frame.
 */
void AnimGraphNode::
stop() {
  FOREACH_INPUT(stop());
}

/**
 * Sets the animation to the indicated frame and holds it there.
 */
void AnimGraphNode::
pose(double frame) {
  FOREACH_INPUT(pose(frame));
}

/**
 * Changes the rate at which the animation plays.  1.0 is the normal speed,
 * 2.0 is twice normal speed, and 0.5 is half normal speed.  0.0 is legal to
 * pause the animation, and a negative value will play the animation
 * backwards.
 */
void AnimGraphNode::
set_play_rate(double play_rate) {
  FOREACH_INPUT(set_play_rate(play_rate));
}

/**
 *
 */
void AnimGraphNode::
wait_pending() {
  FOREACH_INPUT(wait_pending());
}

/**
 *
 */
void AnimGraphNode::
mark_channels(bool frame_blend_flag) {
  FOREACH_INPUT(mark_channels(frame_blend_flag));
}

/**
 *
 */
bool AnimGraphNode::
channel_has_changed(AnimChannelBase *chan, bool frame_blend_flag) const {
  Inputs::const_iterator ii;
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) {
    if ((*ii)->channel_has_changed(chan, frame_blend_flag)) {
      return true;
    }
  }

  return false;
}

/**
 * Produces an output value from the given inputs.
 */
void AnimGraphNode::
evaluate(MovingPartMatrix *part, bool frame_blend_flag) {
  Inputs::const_iterator ii;
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) {
    (*ii)->evaluate(part, frame_blend_flag);
  }
}
