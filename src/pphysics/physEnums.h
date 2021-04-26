/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physEnums.h
 * @author brian
 * @date 2021-04-26
 */

#ifndef PHYSENUMS_H
#define PHYSENUMS_H

#include "pandabase.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysEnums {
PUBLISHED:
  enum CallbackEvent {
    CE_wake,
    CE_sleep,
    CE_contact,
    CE_trigger,
    CE_advance,
    CE_constraint_break,
  };

  enum ContactType {
    CT_found = 1 << 0,
    CT_lost = 1 << 1,
    CT_persists = 1 << 2,
    CT_ccd = 1 << 3,
    CT_threshold_force_found = 1 << 4,
    CT_threshold_force_persists = 1 << 5,
    CT_threshold_force_lost = 1 << 6,
  };
};

#endif // PHYSENUMS_H
