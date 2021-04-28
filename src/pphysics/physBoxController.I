/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physBoxController.I
 * @author brian
 * @date 2021-04-28
 */

/**
 *
 */
INLINE void PhysBoxController::
set_half_extents(const LVector3 &half_extents) {
  _controller->setHalfSideExtent(half_extents[0]);
  _controller->setHalfForwardExtent(half_extents[1]);
  _controller->setHalfHeight(half_extents[2]);
}

/**
 *
 */
INLINE LVector3 PhysBoxController::
get_half_extents() const {
  return LVector3(
    _controller->getHalfSideExtent(),
    _controller->getHalfForwardExtent(),
    _controller->getHalfHeight()
  );
}
