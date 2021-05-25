/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animLayer.I
 * @author brian
 * @date 2021-05-24
 */

/**
 *
 */
INLINE PN_stdfloat AnimLayer::
get_fade_out(PN_stdfloat frame_time) const {
  PN_stdfloat s;
  if (_layer_fade_out_time <= 0.0f) {
    s = 0.0f;

  } else {
    // Blend in over 0.2 seconds.
    s = 1.0f - (frame_time - _layer_anim_time) / _layer_fade_out_time;
    if (s > 0.0f && s <= 1.0f) {
      // Do a nice spline curve.
      s = 3 * s * s - 2 * s * s * s;
    } else if (s > 1.0f) {
      // Shouldn't happen, but maybe frame_time is behind anim_time?
      s = 1.0f;
    }
  }
  return s;
}
