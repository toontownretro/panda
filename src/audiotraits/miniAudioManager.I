/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file miniAudioManager.I
 * @author brian
 * @date 2022-09-06
 */

/**
 *
 */
INLINE const LPoint3 &MiniAudioManager::
get_listener_pos() const {
  return _listener_pos;
}

/**
 *
 */
INLINE const LVector3 &MiniAudioManager::
get_listener_forward() const {
  return _listener_forward;
}

/**
 *
 */
INLINE const LVector3 &MiniAudioManager::
get_listener_up() const {
  return _listener_up;
}

/**
 *
 */
INLINE const LVector3 &MiniAudioManager::
get_listener_velocity() const {
  return _listener_velocity;
}
