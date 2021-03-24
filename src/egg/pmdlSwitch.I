/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlSwitch.I
 * @author lachbr
 * @date 2021-02-13
 */

/**
 *
 */
INLINE PMDLSwitch::
PMDLSwitch() {
  _in_distance = 0.0;
  _out_distance = 100000.0;
  _fade = 0.0;
}

/**
 * Sets the distance at which the LOD should be switched in.
 */
INLINE void PMDLSwitch::
set_in_distance(PN_stdfloat distance) {
  _in_distance = distance;
}

/**
 * Returns the distance at which the LOD should be switched in.
 */
INLINE PN_stdfloat PMDLSwitch::
get_in_distance() const {
  return _in_distance;
}

/**
 * Sets the distance at which the LOD should be switched out.
 */
INLINE void PMDLSwitch::
set_out_distance(PN_stdfloat distance) {
  _out_distance = distance;
}

/**
 * Returns the distance at which the LOD should be switched out.
 */
INLINE PN_stdfloat PMDLSwitch::
get_out_distance() const {
  return _out_distance;
}

/**
 * Adds a name of an EggGroup that should be made part of this LOD switch.
 */
INLINE void PMDLSwitch::
add_group(const std::string &group) {
  _groups.push_back(group);
}

/**
 * Returns the number of EggGroups in this switch.
 */
INLINE size_t PMDLSwitch::
get_num_groups() const {
  return _groups.size();
}

/**
 * Returns the nth EggGroup in this switch.
 */
INLINE std::string PMDLSwitch::
get_group(size_t n) const {
  nassertr(n < _groups.size(), std::string());
  return _groups[n];
}

/**
 * Sets the center reference point for computing the LOD distance.
 */
INLINE void PMDLSwitch::
set_center(const LPoint3 &center) {
  _center = center;
}

/**
 * Returns the center reference point for computing the LOD distance.
 */
INLINE const LPoint3 &PMDLSwitch::
get_center() const {
  return _center;
}

/**
 * Sets the time to fade in this switch.
 */
INLINE void PMDLSwitch::
set_fade(PN_stdfloat fade) {
  _fade = fade;
}

/**
 * Returns the time to fade in this switch.
 */
INLINE PN_stdfloat PMDLSwitch::
get_fade() const {
  return _fade;
}
