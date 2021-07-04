/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikChain.I
 * @author lachbr
 * @date 2021-02-11
 */

/**
 * This private constructor only exists for Bam file reading.
 */
INLINE IKChain::
IKChain() {
  _end_joint = -1;
  _middle_joint = -1;
  _top_joint = -1;
  _height = 0.0;
  _floor = 0.0;
  _pad = 0.0;
}

/**
 * Sets the direction of the knee/elbow joint.
 */
INLINE void IKChain::
set_middle_joint_direction(const LVector3 &dir) {
  _middle_direction = dir;
}

/**
 * Returns the direction of the knee/elbow joint.
 */
INLINE const LVector3 &IKChain::
get_middle_joint_direction() const {
  return _middle_direction;
}

/**
 * Sets the center position.
 */
INLINE void IKChain::
set_center(const LPoint3 &center) {
  _center = center;
}

/**
 * Returns the center position.
 */
INLINE const LPoint3 &IKChain::
get_center() const {
  return _center;
}

/**
 * Sets the height.
 */
INLINE void IKChain::
set_height(PN_stdfloat height) {
  _height = height;
}

/**
 * Returns the height.
 */
INLINE PN_stdfloat IKChain::
get_height() const {
  return _height;
}

/**
 * Sets the floor.
 */
INLINE void IKChain::
set_floor(PN_stdfloat floor) {
  _floor = floor;
}

/**
 * Returns the floor.
 */
INLINE PN_stdfloat IKChain::
get_floor() const {
  return _floor;
}

/**
 * Sets the pad.
 */
INLINE void IKChain::
set_pad(PN_stdfloat pad) {
  _pad = pad;
}

/**
 * Returns the pad.
 */
INLINE PN_stdfloat IKChain::
get_pad() const {
  return _pad;
}

/**
 * Returns the foot/hand joint index.
 */
INLINE int IKChain::
get_end_joint() const {
  return _end_joint;
}

/**
 * Returns the knee/elbow joint.
 */
INLINE int IKChain::
get_middle_joint() const {
  return _middle_joint;
}

/**
 * Returns the hip/shoulder joint.
 */
INLINE int IKChain::
get_top_joint() const {
  return _top_joint;
}
