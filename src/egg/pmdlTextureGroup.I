/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmdlTextureGroup.I
 * @author lachbr
 * @date 2021-02-13
 */

/**
 *
 */
INLINE PMDLTextureGroup::
PMDLTextureGroup() {
}

/**
 * Adds a new material to the group.
 */
INLINE void PMDLTextureGroup::
add_material(const Filename &filename) {
  _materials.push_back(filename);
}

/**
 * Returns the number of materials in the group.
 */
INLINE size_t PMDLTextureGroup::
get_num_materials() const {
  return _materials.size();
}

/**
 * Returns the nth material in the group.
 */
INLINE const Filename &PMDLTextureGroup::
get_material(size_t n) const {
  return _materials[n];
}
