/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikTarget.h
 * @author brian
 * @date 2022-01-30
 */

#ifndef IKTARGET_H
#define IKTARGET_H

#include "pandabase.h"
#include "luse.h"

/**
 * Defines a user-controlled target position/orientation for a character's
 * IK chain.
 */
class EXPCL_PANDA_ANIM IKTarget {
PUBLISHED:
  IKTarget() = default;
  IKTarget(const IKTarget &copy) = default;

  void set_matrix(const LMatrix4 &mat) { _matrix = mat; }

  LMatrix4 _matrix;
};

#include "ikTarget.I"

#endif // IKTARGET_H
