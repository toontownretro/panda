/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physx_shaders.I
 * @author brian
 * @date 2021-04-18
 */

/**
 * Enables or disables collisions between the two indicated groups.
 */
INLINE void PandaSimulationFilterShader::
set_group_collision_flag(int group1, int group2, bool enable) {
  _collision_table[group1][group2]._enable_collisions = enable;
  _collision_table[group2][group1]._enable_collisions = enable;
}

/**
 * Returns whether or not the indicated collision groups have collisions
 * enabled between them.
 */
INLINE bool PandaSimulationFilterShader::
get_group_collision_flag(int group1, int group2) {
  return _collision_table[group1][group2]._enable_collisions;
}
