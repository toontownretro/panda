/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneTop.I
 * @author brian
 * @date 2021-11-17
 */

/**
 *
 */
INLINE void SceneTop::
set_vis_info(SceneVisibility *info) {
  _vis_info = info;
}

/**
 *
 */
INLINE SceneVisibility *SceneTop::
get_vis_info() const {
  return _vis_info;
}
