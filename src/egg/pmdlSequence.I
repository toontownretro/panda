/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlSequence.I
 * @author lachbr
 * @date 2021-02-13
 */

/**
 *
 */
PMDLSequence::
PMDLSequence(const std::string &name) :
  PMDLObject(name),
  _fps(30),
  _fade_in(0.2),
  _fade_out(0.2),
  _snap(false)
{
}

/**
 * Sets the filename of the egg file that contains the animation table.
 */
INLINE void PMDLSequence::
set_anim_filename(const Filename &filename) {
  _anim_filename = filename;
}

/**
 * Returns the filename of the egg file that contains the animation table.
 */
INLINE Filename PMDLSequence::
get_anim_filename() const {
  return _anim_filename;
}

/**
 * Sets the frame rate of the sequence.
 */
INLINE void PMDLSequence::
set_fps(int fps) {
  _fps = fps;
}

/**
 * Returns the frame rate of the sequence.
 */
INLINE int PMDLSequence::
get_fps() const {
  return _fps;
}

/**
 * Sets the in transition time of the sequence.
 */
INLINE void PMDLSequence::
set_fade_in(PN_stdfloat fade_in) {
  _fade_in = fade_in;
}

/**
 * Returns the in transition time of the sequence.
 */
INLINE PN_stdfloat PMDLSequence::
get_fade_in() const {
  return _fade_in;
}

/**
 * Sets the out transition time of the sequence.
 */
INLINE void PMDLSequence::
set_fade_out(PN_stdfloat fade_out) {
  _fade_out = fade_out;
}

/**
 * Returns the out transition time of the sequence.
 */
INLINE PN_stdfloat PMDLSequence::
get_fade_out() const {
  return _fade_out;
}

/**
 * Sets whether or not a transition into this sequence should snap rather than
 * smoothly transition.
 */
INLINE void PMDLSequence::
set_snap(bool snap) {
  _snap = snap;
}

/**
 * Returns whether or not a transition into this sequence should snap rather
 * than smoothly transition.
 */
INLINE bool PMDLSequence::
get_snap() const {
  return _snap;
}
