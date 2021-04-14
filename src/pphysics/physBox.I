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
  _geom.halfExtents = physx::PxVec3(
    (physx::PxReal)half_extents[0],
    (physx::PxReal)half_extents[1],
    (physx::PxReal)half_extents[2]
  );
}

/**
 *
 */
INLINE void PhysBox::
set_half_extents(PN_stdfloat hx, PN_stdfloat hy, PN_stdfloat hz) {
  _geom.halfExtents = physx::PxVec3(
    (physx::PxReal)hx,
    (physx::PxReal)hy,
    (physx::PxReal)hz
  );
}

/**
 *
 */
INLINE LVector3 PhysBox::
get_half_extents() const {
  return LVector3(
    (PN_stdfloat)_geom.halfExtents.x,
    (PN_stdfloat)_geom.halfExtents.y,
    (PN_stdfloat)_geom.halfExtents.z
  );
}

/**
 *
 */
INLINE bool PhysBox::
is_valid() const {
  return _geom.isValid();
}
