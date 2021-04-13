/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animSequence.cxx
 * @author lachbr
 * @date 2021-03-01
 */

#include "animSequence.h"
#include "clockObject.h"

#define OPEN_FOR_EACH_CONTROL \
  for (AnimControls::const_iterator aci = _controls.begin(); \
       aci != _controls.end(); ++aci) { \
    AnimControl *control = (*aci);

#define CLOSE_FOR_EACH_CONTROL }

#define DELEGATE_TO_CONTROL(func) \
  OPEN_FOR_EACH_CONTROL \
    control->func; \
  CLOSE_FOR_EACH_CONTROL

TypeHandle AnimSequence::_type_handle;

/**
 * Runs the entire animation from beginning to end and stops.
 */
void AnimSequence::
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
void AnimSequence::
play(double from, double to) {
  DELEGATE_TO_CONTROL(play(from, to));
}

/**
 * Starts the entire animation looping.  If restart is true, the animation is
 * restarted from the beginning; otherwise, it continues from the current
 * frame.
 */
void AnimSequence::
loop(bool restart) {
  DELEGATE_TO_CONTROL(loop(restart));
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * indefinitely.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimSequence::
loop(bool restart, double from, double to) {
  DELEGATE_TO_CONTROL(loop(restart, from, to));
}

/**
 * Starts the entire animation bouncing back and forth between its first frame
 * and last frame.  If restart is true, the animation is restarted from the
 * beginning; otherwise, it continues from the current frame.
 */
void AnimSequence::
pingpong(bool restart) {
  DELEGATE_TO_CONTROL(pingpong(restart));
}

/**
 * Loops the animation from the frame "from" to and including the frame "to",
 * and then back in the opposite direction, indefinitely.
 */
void AnimSequence::
pingpong(bool restart, double from, double to) {
  DELEGATE_TO_CONTROL(pingpong(restart, from, to));
}

/**
 * Stops a currently playing or looping animation right where it is.  The
 * animation remains posed at the current frame.
 */
void AnimSequence::
stop() {
  DELEGATE_TO_CONTROL(stop());
}

/**
 * Sets the animation to the indicated frame and holds it there.
 */
void AnimSequence::
pose(double frame) {
  DELEGATE_TO_CONTROL(pose(frame));
}

/**
 * Changes the rate at which the animation plays.  1.0 is the normal speed,
 * 2.0 is twice normal speed, and 0.5 is half normal speed.  0.0 is legal to
 * pause the animation, and a negative value will play the animation
 * backwards.
 */
void AnimSequence::
set_play_rate(double play_rate) {
  DELEGATE_TO_CONTROL(set_play_rate(play_rate));
}

/**
 *
 */
double AnimSequence::
get_play_rate() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_play_rate();
}

/**
 *
 */
double AnimSequence::
get_frame_rate() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_frame_rate();
}

/**
 *
 */
int AnimSequence::
get_num_frames() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_num_frames();
}

/**
 *
 */
int AnimSequence::
get_frame() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_frame();
}

/**
 *
 */
int AnimSequence::
get_next_frame() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_next_frame();
}

/**
 *
 */
double AnimSequence::
get_frac() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_frac();
}

/**
 *
 */
int AnimSequence::
get_full_frame() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_full_frame();
}

/**
 *
 */
double AnimSequence::
get_full_fframe() const {
  nassertr(_effective_control != nullptr, 0);

  return _effective_control->get_full_fframe();
}

/**
 *
 */
bool AnimSequence::
is_playing() const {
  nassertr(_effective_control != nullptr, false);

  return _effective_control->is_playing();
}

/**
 *
 */
void AnimSequence::
evaluate(AnimGraphEvalContext &context) {
  nassertv(_base != nullptr);

  _base->evaluate(context);
}

/**
 *
 */
void AnimSequence::
compute_effective_control() {
  _effective_control = nullptr;
  _controls.clear();

  r_compute_effective_control(this);
}

/**
 *
 */
void AnimSequence::
r_compute_effective_control(AnimGraphNode *node) {
  if (node->is_of_type(AnimControl::get_class_type())) {
    AnimControl *ac = DCAST(AnimControl, node);

    if (_effective_control == nullptr ||
        _effective_control->get_num_frames() < ac->get_num_frames()) {
      _effective_control = ac;
    }

    _controls.push_back(ac);

    return;
  }

  for (int i = 0; i < node->get_num_children(); i++) {
    AnimGraphNode *child = node->get_child(i);
    r_compute_effective_control(child);
  }
}