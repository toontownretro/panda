/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physCapsuleController.I
 * @author brian
 * @date 2023-01-04
 */

/**
 *
 */
INLINE void PhysCapsuleController::
set_size(PN_stdfloat radius, PN_stdfloat height) {
  _controller->setRadius(panda_length_to_physx(radius));
  _controller->setHeight(panda_length_to_physx(height - radius * 2));
}

/**
 *
 */
INLINE PN_stdfloat PhysCapsuleController::
get_radius() const {
  return physx_length_to_panda(_controller->getRadius());
}

/**
 *
 */
INLINE PN_stdfloat PhysCapsuleController::
get_height() const {
  return physx_length_to_panda(_controller->getHeight() + _controller->getRadius() * 2);
}
