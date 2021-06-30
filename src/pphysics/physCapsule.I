/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physCapsule.I
 * @author brian
 * @date 2021-04-14
 */


/**
 *
 */
INLINE void PhysCapsule::
set_radius(PN_stdfloat radius) {
  _geom.radius = panda_length_to_physx(radius);
}

/**
 *
 */
INLINE PN_stdfloat PhysCapsule::
get_radius() const {
  return physx_length_to_panda(_geom.radius);
}

/**
 *
 */
INLINE void PhysCapsule::
set_half_height(PN_stdfloat half_height) {
  _geom.halfHeight = panda_length_to_physx(half_height);
}

/**
 *
 */
INLINE PN_stdfloat PhysCapsule::
get_half_height() const {
  return physx_length_to_panda(_geom.halfHeight);
}

/**
 *
 */
INLINE bool PhysCapsule::
is_valid() const {
  return _geom.isValid();
}
