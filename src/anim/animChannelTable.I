/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelTable.I
 * @author brian
 * @date 2021-08-04
 */

/**
 *
 */
INLINE AnimChannelTable::
AnimChannelTable(const std::string &name, PN_stdfloat fps, int num_frames) :
  AnimChannel(name) {
  _fps = fps;
  _num_frames = num_frames;
  _num_joint_entries = 0;
  _num_slider_entries = 0;
  _root_motion_vector.fill(0.0f);
  //_is_loaded = true;
}

#if 0
/**
 * Initializes an AnimChannelTable with a reference to a Filename that contains the
 * actual animation data.
 */
INLINE AnimChannelTable::
AnimChannelTable(const Filename &filename) :
  AnimChannel("bundle") {
  _filename = filename;
  _is_loaded = false;
}

/**
 * Sets the Filename that the animation data for the AnimChannelTable can be found
 * in.  When this is set, the given Filename will be loaded up to extract the
 * actual animation data for the bundle the first time that the bundle is
 * referenced in a Character.
 */
INLINE void AnimChannelTable::
set_filename(const Filename &filename) {
  _filename = filename;
  _is_loaded = filename.empty();
}

/**
 * Returns the Filename that the animation data for the AnimChannelTable can be
 * found in.  See set_filename().
 */
INLINE const Filename &AnimChannelTable::
get_filename() const {
  return _filename;
}

/**
 * Sets whether or not the actual animation data for the AnimChannelTable has
 * been loaded.
 */
INLINE void AnimChannelTable::
set_loaded(bool flag) {
  _is_loaded = flag;
}

/**
 * Returns true if the actual animation data for the AnimChannelTable has been
 * loaded, or false otherwise.
 */
INLINE bool AnimChannelTable::
is_loaded() const {
  return _is_loaded;
}
#endif

/**
 *
 */
INLINE AnimChannelTable::
AnimChannelTable() :
  AnimChannel("table") {
  //_is_loaded = false;
  _root_motion_vector.fill(0.0f);
}

/**
 *
 */
INLINE AnimChannelTable::
AnimChannelTable(const AnimChannelTable &copy) :
  AnimChannel(copy),
  _joint_frames(copy._joint_frames),
  _slider_table(copy._slider_table),
  _num_joint_entries(copy._num_joint_entries),
  _num_slider_entries(copy._num_slider_entries),
  _root_motion_vector(copy._root_motion_vector)
{
  std::copy(copy._joint_entries, copy._joint_entries + copy._num_joint_entries, _joint_entries);
  std::copy(copy._slider_entries, copy._slider_entries + copy._num_slider_entries, _slider_entries);
}

/**
 *
 */
INLINE void AnimChannelTable::
set_joint_table(JointFrames &&table) {
  _joint_frames = std::move(table);
}


/**
 *
 */
INLINE const JointFrames &AnimChannelTable::
get_joint_table() const {
  return _joint_frames;
}

/**
 *
 */
INLINE const JointFrame &AnimChannelTable::
get_joint_frame(int joint, int frame) const {
  return get_joint_frame(_joint_entries[joint], frame);
}

/**
 *
 */
INLINE const JointFrame &AnimChannelTable::
get_joint_frame(const JointEntry &joint, int frame) const {
  return _joint_frames[joint.first_frame + (frame % joint.num_frames)];
}

/**
 *
 */
INLINE void AnimChannelTable::
set_slider_table(vector_stdfloat &&table) {
  _slider_table = std::move(table);
}

/**
 *
 */
INLINE const vector_stdfloat &AnimChannelTable::
get_slider_table() const {
  return _slider_table;
}

/**
 *
 */
INLINE void AnimChannelTable::
get_scalar(int slider, int frame, PN_stdfloat &scalar) const {
  get_scalar(_slider_entries[slider], frame, scalar);
}

/**
 *
 */
INLINE void AnimChannelTable::
get_scalar(const SliderEntry &slider, int frame, PN_stdfloat &scalar) const {
  scalar = _slider_table[slider.first_frame + (frame % slider.num_frames)];
}

/**
 * Records the name of indicated joint channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimChannelTable::
add_joint_entry(const JointEntry &entry) {
  nassertv(_num_joint_entries < max_character_joints);
  _joint_entries[_num_joint_entries++] = entry;
}

/**
 *
 */
INLINE const JointEntry &AnimChannelTable::
get_joint_entry(int joint) const {
  assert(joint >= 0 && joint < (int)_num_joint_entries);
  return _joint_entries[joint];
}

/**
 * Records the name of indicated slider channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimChannelTable::
add_slider_entry(const SliderEntry &entry) {
  nassertv(_num_slider_entries < max_character_joints);
  _slider_entries[_num_slider_entries++] = entry;
}

/**
 *
 */
INLINE const SliderEntry &AnimChannelTable::
get_slider_entry(int slider) const {
  assert(slider >= 0 && slider < (int)_num_slider_entries);
  return _slider_entries[slider];
}

/**
 * Returns the number of recorded joint channels.
 */
INLINE int AnimChannelTable::
get_num_joint_entries() const {
  return _num_joint_entries;
}

/**
 * Returns the number of recorded slider channels.
 */
INLINE int AnimChannelTable::
get_num_slider_entries() const {
  return _num_slider_entries;
}
