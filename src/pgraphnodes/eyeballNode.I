/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeballNode.I
 * @author lachbr
 * @date 2021-03-24
 */

/**
 *
 */
INLINE void EyeballNode::
set_view_target(const LPoint3 &target) {
  _view_target = target;
}

/**
 *
 */
INLINE void EyeballNode::
set_eye_shift(const LVector3 &shift) {
  _eye_shift = shift;
}

/**
 *
 */
INLINE void EyeballNode::
set_z_offset(PN_stdfloat offset) {
  _z_offset = offset;
}

/**
 *
 */
INLINE void EyeballNode::
set_radius(PN_stdfloat radius) {
  _radius = radius;
}

/**
 *
 */
INLINE void EyeballNode::
set_iris_scale(PN_stdfloat scale) {
  _iris_scale = scale;
}

/**
 *
 */
INLINE void EyeballNode::
set_eye_size(PN_stdfloat size) {
  _eye_size = size;
}

/**
 *
 */
INLINE void EyeballNode::
set_debug_enabled(bool enable) {
  _debug_enabled = enable;
}
