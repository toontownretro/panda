/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spotlightBeam.I
 * @author brian
 * @date 2022-09-28
 */

/**
 *
 */
INLINE void SpotlightBeam::
set_beam_color(const LColor &color) {
  _beam_color = color;
}

/**
 *
 */
INLINE LColor SpotlightBeam::
get_beam_color() const {
  return _beam_color;
}

/**
 *
 */
INLINE void SpotlightBeam::
set_beam_size(PN_stdfloat length, PN_stdfloat width) {
  _beam_length = length;
  _beam_width = width;
}

/**
 *
 */
INLINE PN_stdfloat SpotlightBeam::
get_beam_length() const {
  return _beam_length;
}

/**
 *
 */
INLINE PN_stdfloat SpotlightBeam::
get_beam_width() const {
  return _beam_width;
}

/**
 *
 */
INLINE void SpotlightBeam::
set_halo_color(const LColor &color) {
  _halo_color = color;
}

/**
 *
 */
INLINE LColor SpotlightBeam::
get_halo_color() const {
  return _halo_color;
}

/**
 *
 */
INLINE void SpotlightBeam::
set_halo_size(PN_stdfloat size) {
  _halo_size = size;
}

/**
 *
 */
INLINE PN_stdfloat SpotlightBeam::
get_halo_size() const {
  return _halo_size;
}
