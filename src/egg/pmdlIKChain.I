/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlIKChain.I
 * @author lachbr
 * @date 2021-02-13
 */

/**
 *
 */
INLINE PMDLIKChain::
PMDLIKChain(const std::string &name) :
  PMDLObject(name),
  _height(0.0),
  _floor(0.0),
  _pad(0.0) {
}

/**
 *
 */
INLINE void PMDLIKChain::
set_foot_joint(const std::string &name) {
  _foot_joint = name;
}

/**
 *
 */
INLINE std::string PMDLIKChain::
get_foot_joint() const {
  return _foot_joint;
}

/**
 *
 */
INLINE void PMDLIKChain::
set_knee_direction(const LVector3 &dir) {
  _knee_direction = dir;
}

/**
 *
 */
INLINE LVector3 PMDLIKChain::
get_knee_direction() const {
  return _knee_direction;
}

/**
 *
 */
INLINE void PMDLIKChain::
set_center(const LPoint3 &center) {
  _center = center;
}

/**
 *
 */
INLINE LPoint3 PMDLIKChain::
get_center() const {
  return _center;
}

/**
 *
 */
INLINE void PMDLIKChain::
set_height(PN_stdfloat height) {
  _height = height;
}

/**
 *
 */
INLINE PN_stdfloat PMDLIKChain::
get_height() const {
  return _height;
}

/**
 *
 */
INLINE void PMDLIKChain::
set_floor(PN_stdfloat floor) {
  _floor = floor;
}

/**
 *
 */
INLINE PN_stdfloat PMDLIKChain::
get_floor() const {
  return _floor;
}

/**
 *
 */
INLINE void PMDLIKChain::
set_pad(PN_stdfloat pad) {
  _pad = pad;
}

/**
 *
 */
INLINE PN_stdfloat PMDLIKChain::
get_pad() const {
  return _pad;
}
