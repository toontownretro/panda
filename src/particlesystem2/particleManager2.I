/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleManager2.I
 * @author brian
 * @date 2022-07-31
 */

/**
 *
 */
INLINE ParticleManager2::
ParticleManager2() :
  _removed_systems_lock("removed-systems-lock")
{
}

/**
 *
 */
INLINE ParticleManager2 *ParticleManager2::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new ParticleManager2;
  }
  return _global_ptr;
}
