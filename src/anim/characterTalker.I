/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterTalker.I
 * @author brian
 * @date 2022-02-10
 */

/**
 * Sets the character whose sliders should be moved around with speech.
 */
INLINE void CharacterTalker::
set_character(Character *character) {
  _character = character;
}

/**
 * Returns the character whose sliders are being moved around with speech.
 */
INLINE Character *CharacterTalker::
get_character() const {
  return _character;
}
