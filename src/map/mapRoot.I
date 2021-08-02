/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRoot.I
 * @author brian
 * @date 2021-07-09
 */

/**
 *
 */
INLINE MapData *MapRoot::
get_data() const {
  return _data;
}

/**
 * Sets whether or not the node should only render child mesh groups that are
 * in the potentially visible set.
 */
INLINE void MapRoot::
set_pvs_cull(bool flag) {
  _pvs_cull = flag;
}

/**
 * Returns true if the node shouuld only render child mesh groups that are in
 * the potentially visible set.
 */
INLINE bool MapRoot::
get_pvs_cull() const {
  return _pvs_cull;
}
