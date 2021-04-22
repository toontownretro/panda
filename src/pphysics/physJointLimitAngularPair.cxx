/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitAngularPair.cxx
 * @author brian
 * @date 2021-04-22
 */

#include "physJointLimitAngularPair.h"

/**
 *
 */
physx::PxJointLimitParameters &PhysJointLimitAngularPair::
get_params() {
  return _limit;
}
