/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimit.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSJOINTLIMIT_H
#define PHYSJOINTLIMIT_H

#include "pandabase.h"
#include "numeric_types.h"
#include "physx_includes.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysJointLimit {
PUBLISHED:
  INLINE void set_restitution(PN_stdfloat restitution);
  INLINE PN_stdfloat get_restitution() const;

  INLINE void set_bounce_threshold(PN_stdfloat threshold);
  INLINE PN_stdfloat get_bounce_threshold() const;

  INLINE void set_stiffness(PN_stdfloat stiff);
  INLINE PN_stdfloat get_stiffness() const;

  INLINE void set_damping(PN_stdfloat damp);
  INLINE PN_stdfloat get_damping() const;

  INLINE void set_contact_distance(PN_stdfloat dist);
  INLINE PN_stdfloat get_contact_distance() const;

  INLINE bool is_valid() const;

  INLINE bool is_soft() const;

protected:
  INLINE const physx::PxJointLimitParameters &get_params() const;
  virtual physx::PxJointLimitParameters &get_params() = 0;
};

#include "physJointLimit.I"

#endif // PHYSJOINTLIMIT_H
