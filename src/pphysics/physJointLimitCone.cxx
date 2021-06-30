/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitCone.cxx
 * @author brian
 * @date 2021-04-22
 */

#include "physJointLimitCone.h"

/**
 *
 */
PhysJointLimitCone::
PhysJointLimitCone(PN_stdfloat y_limit, PN_stdfloat z_limit, PN_stdfloat contact_distance) :
  _limit(deg_2_rad(y_limit), deg_2_rad(z_limit),
         contact_distance != -1.0f ? panda_ang_to_physx(contact_distance) : contact_distance)
{
}

/**
 *
 */
physx::PxJointLimitParameters &PhysJointLimitCone::
get_params() {
  return _limit;
}
