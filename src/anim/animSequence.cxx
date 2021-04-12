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
#include "animControl.h"
#include "animSampleNode.h"
#include "clockObject.h"

/**
 *
 */
void AnimSequence::
play() {
  _play_mode = PM_play;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = -1;
  _end_frame = -1;
  _restart = false;
}

/**
 *
 */
void AnimSequence::
play(double from, double to) {
  _play_mode = PM_play;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = from;
  _end_frame = to;
  _restart = false;
}

/**
 *
 */
void AnimSequence::
loop(bool restart) {
  _play_mode = PM_loop;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = -1;
  _end_frame = -1;
  _restart = restart;
}

/**
 *
 */
void AnimSequence::
loop(bool restart, double from, double to) {
  _play_mode = PM_loop;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = from;
  _end_frame = to;
  _restart = restart;
}

/**
 *
 */
void AnimSequence::
pingpong(bool restart) {
  _play_mode = PM_pingpong;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = -1;
  _end_frame = -1;
  _restart = restart;
}

/**
 *
 */
void AnimSequence::
pingpong(bool restart, double from, double to) {
  _play_mode = PM_pingpong;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = from;
  _end_frame = to;
  _restart = restart;
}

/**
 *
 */
void AnimSequence::
pose(double frame) {
  _play_mode = PM_pose;
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _start_frame = frame;
  _end_frame = frame;
}

/**
 *
 */
void AnimSequence::
stop() {
  _play_mode = PM_pose;
}

/**
 *
 */
void AnimSequence::
compute_effective_num_frames() {
  _effective_num_frames = 0;

  r_compute_effective_num_frames(this);
}
