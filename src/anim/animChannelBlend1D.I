/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBlend1D.I
 * @author brian
 * @date 2021-08-04
 */

/**
 * Sets the index of the PoseParameter that should be used to control the
 * blending of the channels within the multi-channel.
 */
INLINE void AnimChannelBlend1D::
set_blend_param(int param) {
  _blend_param = param;
}

/**
 * Returns the index of the PoseParameter that is being used to control the
 * blending of the channels within the multi-channel.
 */
INLINE int AnimChannelBlend1D::
get_blend_param() const {
  return _blend_param;
}
