/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathutil_misc.T
 * @author brian
 * @date 2020-10-19
 *
 * Miscellaneous math utility functions.
 */

// Returns A + (B-A)*flPercent.
// float Lerp( float flPercent, float A, float B );
template <class T>
INLINE T
tlerp(float percent, T const &A, T const &B) {
  return A + (B - A) * percent;
}
