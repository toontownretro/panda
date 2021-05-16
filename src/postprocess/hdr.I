/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hdr.I
 * @author lachbr
 * @date 2020-12-08
 */

/**
 * Returns the current average luminance of the scene.
 * Note: This is the time-averaged value, not instantaneous.
 */
INLINE float HDRPass::
get_luminance() const {
  return _luminance;
}

/**
 * Returns the current camera aperature size.
 */
INLINE float HDRPass::
get_aperature() const {
  return _aperature;
}

/**
 * Returns the current camera shutter speed.
 */
INLINE float HDRPass::
get_shutter_speed() const {
  return _shutter_speed;
}

/**
 * Returns the current camera ISO value.
 */
INLINE float HDRPass::
get_iso() const {
  return _iso;
}

/**
 * Returns the current maximum camera luminance.
 */
INLINE float HDRPass::
get_max_luminance() const {
  return _max_luminance;
}

/**
 * Returns the current camera linear exposure value.
 */
INLINE float HDRPass::
get_exposure() const {
  return _exposure;
}

/**
 * Returns the current camera exposure value expressed in EV units (exp2 of
 * linear exposure).
 */
INLINE float HDRPass::
get_exposure_value() const {
  return _exposure_value;
}
