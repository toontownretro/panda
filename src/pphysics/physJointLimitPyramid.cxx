/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitPyramid.cxx
 * @author brian
 * @date 2021-04-23
 */

#include "physJointLimitPyramid.h"

/**
 *
 */
physx::PxJointLimitParameters &PhysJointLimitPyramid::
get_params() {
  return _limit;
}
