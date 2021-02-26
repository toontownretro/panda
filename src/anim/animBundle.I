/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundle.I
 * @author drose
 * @date 1999-02-21
 */

/**
 *
 */
INLINE AnimBundle::
AnimBundle(const std::string &name, PN_stdfloat fps, int num_frames) :
  Namable(name) {
  _fps = fps;
  _num_frames = num_frames;
}

/**
 *
 */
INLINE AnimBundle::
AnimBundle() {
}

/**
 * Sets the ideal number of frames per second of the animation, when it is
 * running at normal speed.  This may not be the same as the actual playing
 * frame rate, as it might have been adjusted through set_play_rate() on the
 * AnimControl object.  See AnimControl::get_effective_frame_rate().
 */
INLINE void AnimBundle::
set_base_frame_rate(PN_stdfloat fps) {
  _fps = fps;
}

/**
 * Returns the ideal number of frames per second of the animation, when it is
 * running at normal speed.  This may not be the same as the actual playing
 * frame rate, as it might have been adjusted through set_play_rate() on the
 * AnimControl object.  See AnimControl::get_effective_frame_rate().
 */
INLINE PN_stdfloat AnimBundle::
get_base_frame_rate() const {
  return _fps;
}

/**
 * Sets the number of frames of animation, or 0 if the animation has no fixed
 * number of frames.
 */
INLINE void AnimBundle::
set_num_frames(int num_frames) {
  _num_frames = num_frames;
}

/**
 * Returns the number of frames of animation, or 0 if the animation has no
 * fixed number of frames.
 */
INLINE int AnimBundle::
get_num_frames() const {
  return _num_frames;
}

/**
 * Sets the joint animation table.
 */
INLINE void AnimBundle::
set_joint_channel_data(CPTA_JointFrameData data) {
  _joint_frames = data;
}

/**
 * Returns the joint animation table.
 */
INLINE CPTA_JointFrameData AnimBundle::
get_joint_channel_data() const {
  return _joint_frames;
}

/**
 * Sets the slider animation table.
 */
INLINE void AnimBundle::
set_slider_channel_data(CPTA_stdfloat data) {
  _slider_frames = data;
}

/**
 * Returns the slider animation table.
 */
INLINE CPTA_stdfloat AnimBundle::
get_slider_channel_data() const {
  return _slider_frames;
}

/**
 * Records the name of indicated joint channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimBundle::
record_joint_channel_name(int channel, const std::string &name) {
  _joint_channel_names[name] = channel;
}

/**
 * Records the name of indicated slider channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimBundle::
record_slider_channel_name(int channel, const std::string &name) {
  _slider_channel_names[name] = channel;
}

/**
 * Returns the number of recorded joint channels.
 */
INLINE int AnimBundle::
get_num_joint_channels() const {
  return (int)_joint_channel_names.size();
}

/**
 * Returns the number of recorded slider channels.
 */
INLINE int AnimBundle::
get_num_slider_channels() const {
  return (int)_slider_channel_names.size();
}

/**
 * Returns the index into the animation table for the animation data of the
 * given joint at the given frame.
 */
INLINE int AnimBundle::
get_channel_data_index(int num_channels, int frame, int channel) {
  return num_channels * frame + channel;
}
