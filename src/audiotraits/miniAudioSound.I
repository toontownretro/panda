/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file miniAudioSound.I
 * @author brian
 * @date 2022-09-06
 */

/**
 *
 */
INLINE const LPoint3 &MiniAudioSound::
get_pos() const {
  return _pos;
}

/**
 *
 */
INLINE const LVector3 &MiniAudioSound::
get_velocity() const {
  return _velocity;
}

/**
 *
 */
INLINE const LVector3 &MiniAudioSound::
get_forward() const {
  return _forward;
}

/**
 *
 */
INLINE const LVector3 &MiniAudioSound::
get_up() const {
  return _up;
}

/**
 *
 */
INLINE MiniAudioManager *MiniAudioSound::
get_manager() const {
  return _mgr;
}
