/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialGroup.I
 * @author lachbr
 * @date 2021-02-14
 */

/**
 * Adds a new RenderState to the group.
 */
INLINE void MaterialGroup::
add_material(const RenderState *state) {
  _states.push_back(state);
}

/**
 * Returns the number of RenderStates in the group.
 */
INLINE size_t MaterialGroup::
get_num_materials() const {
  return _states.size();
}

/**
 * Returns the nth RenderState in the group.
 */
INLINE const RenderState *MaterialGroup::
get_material(size_t n) const {
  nassertr(n < _states.size(), nullptr);
  return _states[n];
}

/**
 * Removes all RenderStates from the group.
 */
INLINE void MaterialGroup::
clear_materials() {
  _states.clear();
}
