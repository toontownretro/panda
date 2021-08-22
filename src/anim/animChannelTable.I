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
  _has_character_bound = false;
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
}

/**
 *
 */
INLINE AnimChannelTable::
AnimChannelTable(const AnimChannelTable &copy) :
  AnimChannel(copy),
  _joint_entries(copy._joint_entries),
  _joint_frames(copy._joint_frames),
  _slider_entries(copy._slider_entries),
  _slider_table(copy._slider_table),
  _joint_map(copy._joint_map),
  _slider_map(copy._slider_map),
  _has_character_bound(copy._has_character_bound)
{
}

/**
 *
 */
INLINE void AnimChannelTable::
set_joint_table(JointFrames table) {
  _joint_frames = table;
}


/**
 *
 */
INLINE JointFrames AnimChannelTable::
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
set_slider_table(PTA_stdfloat table) {
  _slider_table = table;
}

/**
 *
 */
INLINE PTA_stdfloat AnimChannelTable::
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
  _joint_entries.push_back(entry);
}

/**
 *
 */
INLINE const JointEntry &AnimChannelTable::
get_joint_entry(int joint) const {
  return _joint_entries[joint];
}

/**
 * Records the name of indicated slider channel.  This is used to aid Character
 * in binding to the animation, because we can't guarantee ordering of
 * channels.
 */
INLINE void AnimChannelTable::
add_slider_entry(const SliderEntry &entry) {
  _slider_entries.push_back(entry);
}

/**
 *
 */
INLINE const SliderEntry &AnimChannelTable::
get_slider_entry(int slider) const {
  return _slider_entries[slider];
}

/**
 * Returns the number of recorded joint channels.
 */
INLINE int AnimChannelTable::
get_num_joint_entries() const {
  return (int)_joint_entries.size();
}

/**
 * Returns the number of recorded slider channels.
 */
INLINE int AnimChannelTable::
get_num_slider_entries() const {
  return (int)_slider_entries.size();
}

/**
 * Initializes the mapping of character joint indices to animation joint
 * indices with the indicated number of character joints.
 */
INLINE void AnimChannelTable::
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
INLINE void AnimChannelTable::
map_character_joint_to_anim_joint(int character_joint, int anim_joint) {
  _joint_map[character_joint] = anim_joint;
}

/**
 *
 */
INLINE void AnimChannelTable::
map_character_slider_to_anim_slider(int character_slider, int anim_slider) {
  _slider_map[character_slider] = anim_slider;
}

/**
 * Returns the animation joint index associated with the indicated character
 * joint index.
 */
INLINE int AnimChannelTable::
get_anim_joint_for_character_joint(int character_joint) const {
  return _joint_map[character_joint];
}

/**
 *
 */
INLINE int AnimChannelTable::
get_anim_slider_for_character_slider(int character_slider) const {
  return _slider_map[character_slider];
}

/**
 * Returns true if the AnimChannelTable has a valid mapping to a Character, or false
 * otherwise.
 */
INLINE bool AnimChannelTable::
has_mapped_character() const {
  return _has_character_bound;
}
