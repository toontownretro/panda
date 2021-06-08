/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displayRegionDrawCallbackData.I
 * @author drose
 * @date 2009-03-13
 */

/**
 * Returns a pointer to the CullResult, the list of CullableObjects that
 * should be drawn in this DisplayRegion.
 */
INLINE CullResult *DisplayRegionDrawCallbackData::
get_cull_result() const {
  return _cull_result;
}

/**
 * Returns a pointer to the SceneSetup object, which contains information
 * about the camera and such.
 */
INLINE SceneSetup *DisplayRegionDrawCallbackData::
get_scene_setup() const {
  return _scene_setup;
}

/**
 * Sets the lost_state flag.  If this is true, the callback does not have to
 * be quite so careful to clean up after itself; Panda will assume that the
 * graphics state is in an unknown state after the callback has finished, and
 * will issue all the necessary calls to restore it.  If this is false, Panda
 * will assume the callback will leave the graphics state exactly as it came
 * in, and won't bother to try to restore it.  The default is true.
 */
INLINE void DisplayRegionDrawCallbackData::
set_lost_state(bool lost_state) {
  _lost_state = lost_state;
}

/**
 * Returns the lost_state flag.  See set_lost_state().
 */
INLINE bool DisplayRegionDrawCallbackData::
get_lost_state() const {
  return _lost_state;
}
