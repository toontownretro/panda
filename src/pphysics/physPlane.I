/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physPlane.I
 * @author brian
 * @date 2021-04-14
 */

/**
 *
 */
INLINE void PhysPlane::
set_plane(const LPlane &plane) {
  _plane = plane;
}

/**
 *
 */
INLINE const LPlane &PhysPlane::
get_plane() const {
  return _plane;
}

/**
 *
 */
INLINE bool PhysPlane::
is_valid() const {
  return _geom.isValid();
}
