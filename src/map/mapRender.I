/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRender.I
 * @author brian
 * @date 2021-10-15
 */

/**
 *
 */
INLINE void MapRender::
set_map_data(MapData *data) {
  _map_data = data;
}

/**
 *
 */
INLINE void MapRender::
clear_map_data() {
  _map_data = nullptr;
}

/**
 *
 */
INLINE MapData *MapRender::
get_map_data() const {
  return _map_data;
}
