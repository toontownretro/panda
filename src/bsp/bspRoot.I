/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspRoot.I
 * @author lachbr
 * @date 2021-01-02
 */

/**
 *
 */
INLINE BSPRoot::
BSPRoot(const std::string &name) :
  PandaNode(name) {
}

/**
 * Sets the underlying BSPData associated with this node.
 */
INLINE void BSPRoot::
set_bsp_data(BSPData *data) {
  _data = data;
}

/**
 * Returns the underlying BSPData associated with this node.
 */
INLINE BSPData *BSPRoot::
get_bsp_data() const {
  return _data;
}
