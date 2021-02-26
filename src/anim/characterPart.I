/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterPart.I
 * @author lachbr
 * @date 2021-02-23
 */

/**
 *
 */
INLINE int CharacterPart::
get_max_bound() const {
  return (int)_channels.size();
}

/**
 *
 */
INLINE int CharacterPart::
get_bound(int n) const {
  return _channels[n];
}
