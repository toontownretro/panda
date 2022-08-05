/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleInitializer2.I
 * @author brian
 * @date 2022-04-04
 */

/**
 *
 */
INLINE void P2_INIT_RemapAttribute::
set_mode(Mode mode) {
  _mode = mode;
}

/**
 *
 */
INLINE P2_INIT_RemapAttribute::Mode P2_INIT_RemapAttribute::
get_mode() const {
  return _mode;
}

/**
 *
 */
INLINE void P2_INIT_RemapAttribute::
set_spline(bool flag) {
  _spline = flag;
}

/**
 *
 */
INLINE bool P2_INIT_RemapAttribute::
get_spline() const {
  return _spline;
}
