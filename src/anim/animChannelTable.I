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
  AnimChannel(name),
  _table_flags(TF_none)
{
  _fps = fps;
  _num_frames = num_frames;
  _root_motion_vector.fill(0.0f);
  _frames.resize(num_frames);
  _slider_frames.resize(num_frames);
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
  AnimChannel("table"),
  _table_flags(TF_none)
{
  //_is_loaded = false;
  _root_motion_vector.fill(0.0f);
}

/**
 *
 */
INLINE AnimChannelTable::
AnimChannelTable(const AnimChannelTable &copy) :
  AnimChannel(copy),
  _joint_names(copy._joint_names),
  _joint_formats(copy._joint_formats),
  _frames(copy._frames),
  _slider_names(copy._slider_names),
  _slider_formats(copy._slider_formats),
  _slider_frames(copy._slider_frames),
  _root_motion_vector(copy._root_motion_vector),
  _table_flags(copy._table_flags)
{
}

/**
 *
 */
INLINE void AnimChannelTable::
set_joint_table(FrameDatas &&table) {
  _frames = std::move(table);
}


/**
 *
 */
INLINE const FrameDatas &AnimChannelTable::
get_joint_table() const {
  return _frames;
}

/**
 *
 */
INLINE void AnimChannelTable::
set_joint_names(vector_string &&names) {
  _joint_names = std::move(names);
}

/**
 *
 */
INLINE const vector_string &AnimChannelTable::
get_joint_names() const {
  return _joint_names;
}

/**
 * Returns the number of recorded joint channels.
 */
INLINE int AnimChannelTable::
get_num_joint_entries() const {
  return (int)_joint_names.size();
}

/**
 * Returns the number of recorded slider channels.
 */
INLINE int AnimChannelTable::
get_num_slider_entries() const {
  return (int)_slider_names.size();
}
