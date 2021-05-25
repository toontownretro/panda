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
  AnimGraphNode(name) {
  _fps = fps;
  _num_frames = num_frames;
  _has_character_bound = false;
}

/**
 *
 */
INLINE AnimBundle::
AnimBundle() :
  AnimGraphNode("bundle") {
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
 *
 */
INLINE void AnimBundle::
set_joint_table(JointFrames &&table) {
  _joint_frames = std::move(table);
}


/**
 *
 */
INLINE const JointFrames &AnimBundle::
get_joint_table() const {
  return _joint_frames;
}

/**
 *
 */
INLINE const JointFrame &AnimBundle::
get_joint_frame(int joint, int frame) const {
  return get_joint_frame(_joint_entries[joint], frame);
}

/**
 *
 */
INLINE const JointFrame &AnimBundle::
get_joint_frame(const JointEntry &joint, int frame) const {
  return _joint_frames[joint.first_frame + (frame % joint.num_frames)];
}

/**
 *
 */
INLINE void AnimBundle::
set_slider_table(vector_stdfloat &&table) {
  _slider_table = std::move(table);
}

/**
 *
 */
INLINE const vector_stdfloat &AnimBundle::
get_slider_table() const {
  return _slider_table;
}

/**
 *
 */
INLINE void AnimBundle::
get_scalar(int slider, int frame, PN_stdfloat &scalar) const {
  get_scalar(_slider_entries[slider], frame, scalar);
}

/**
 *
 */
INLINE void AnimBundle::
get_scalar(const SliderEntry &slider, int frame, PN_stdfloat &scalar) const {
  scalar = _slider_table[slider.first_frame + (frame % slider.num_frames)];
}

/**
 * Records the name of indicated joint channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimBundle::
add_joint_entry(const JointEntry &entry) {
  _joint_entries.push_back(entry);
}

/**
 *
 */
INLINE const JointEntry &AnimBundle::
get_joint_entry(int joint) const {
  return _joint_entries[joint];
}

/**
 * Records the name of indicated slider channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimBundle::
add_slider_entry(const SliderEntry &entry) {
  _slider_entries.push_back(entry);
}

/**
 *
 */
INLINE const SliderEntry &AnimBundle::
get_slider_entry(int slider) const {
  return _slider_entries[slider];
}

/**
 * Returns the number of recorded joint channels.
 */
INLINE int AnimBundle::
get_num_joint_entries() const {
  return (int)_joint_entries.size();
}

/**
 * Returns the number of recorded slider channels.
 */
INLINE int AnimBundle::
get_num_slider_entries() const {
  return (int)_slider_entries.size();
}

/**
 * Initializes the mapping of character joint indices to animation joint
 * indices with the indicated number of character joints.
 */
INLINE void AnimBundle::
init_joint_mapping(int num_character_joints, int num_character_sliders) {
  _joint_map.clear();
  _joint_map.resize(num_character_joints);
  _slider_map.clear();
  _slider_map.resize(num_character_sliders);
  for (size_t i = 0; i < _joint_map.size(); i++) {
    _joint_map[i] = -1;
  }
  for (size_t i = 0; i < _slider_map.size(); i++) {
    _slider_map[i] = -1;
  }
  _has_character_bound = true;
}

/**
 * Maps the indicated character joint to a corresponding animation joint.
 */
INLINE void AnimBundle::
map_character_joint_to_anim_joint(int character_joint, int anim_joint) {
  _joint_map[character_joint] = anim_joint;
}

/**
 *
 */
INLINE void AnimBundle::
map_character_slider_to_anim_slider(int character_slider, int anim_slider) {
  _slider_map[character_slider] = anim_slider;
}

/**
 * Returns the animation joint index associated with the indicated character
 * joint index.
 */
INLINE int AnimBundle::
get_anim_joint_for_character_joint(int character_joint) const {
  return _joint_map[character_joint];
}

/**
 *
 */
INLINE int AnimBundle::
get_anim_slider_for_character_slider(int character_slider) const {
  return _slider_map[character_slider];
}

/**
 * Returns true if the AnimBundle has a valid mapping to a Character, or false
 * otherwise.
 */
INLINE bool AnimBundle::
has_mapped_character() const {
  return _has_character_bound;
}
