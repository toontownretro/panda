/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleForce2.I
 * @author brian
 * @date 2022-04-06
 */

/**
 *
 */
INLINE LVector3 ParticleForce2::
apply_axis_mask(const LVector3 &vec) {
  return LVector3(
    ((_axis_mask & AM_x) != 0u) ? vec[0] : 0.0f,
    ((_axis_mask & AM_y) != 0u) ? vec[1] : 0.0f,
    ((_axis_mask & AM_z) != 0u) ? vec[2] : 0.0f
  );
}
