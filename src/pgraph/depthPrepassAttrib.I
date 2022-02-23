/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthPrepassAttrib.I
 * @author brian
 * @date 2022-02-22
 */

/**
 *
 */
INLINE DepthPrepassAttrib::
DepthPrepassAttrib(BitMask32 camera_mask) :
  _camera_mask(camera_mask)
{
}

/**
 *
 */
INLINE BitMask32 DepthPrepassAttrib::
get_camera_mask() const {
  return _camera_mask;
}
