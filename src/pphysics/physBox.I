/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physBox.I
 * @author brian
 * @date 2021-04-14
 */

/**
 *
 */
INLINE void PhysBox::
set_half_extents(const LVector3 &half_extents) {
  _geom.halfExtents = panda_vec_to_physx(half_extents);
}

/**
 *
 */
INLINE void PhysBox::
set_half_extents(PN_stdfloat hx, PN_stdfloat hy, PN_stdfloat hz) {
  _geom.halfExtents = panda_vec_to_physx(LVecBase3(hx, hy, hz));
}

/**
 *
 */
INLINE LVector3 PhysBox::
get_half_extents() const {
  return physx_vec_to_panda(_geom.halfExtents);
}

/**
 *
 */
INLINE bool PhysBox::
is_valid() const {
  return _geom.isValid();
}
