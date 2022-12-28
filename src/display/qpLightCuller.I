/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qpLightCuller.I
 * @author brian
 * @date 2022-12-26
 */

/**
 *
 */
INLINE void qpLightCuller::
set_frustum_div(int x, int y, int z) {
  _x_div = x;
  _y_div = y;
  _z_div = z;
}
