/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitAngularPair.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSJOINTLIMITANGULARPAIR_H
#define PHYSJOINTLIMITANGULARPAIR_H

#include "pandabase.h"
#include "physJointLimit.h"
#include "deg_2_rad.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysJointLimitAngularPair : public PhysJointLimit {
PUBLISHED:
  INLINE PhysJointLimitAngularPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit,
                                   PN_stdfloat contact_dist = -1.0f);
  INLINE PhysJointLimitAngularPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit,
                                   PN_stdfloat stiffness, PN_stdfloat damping);
  INLINE PhysJointLimitAngularPair(const physx::PxJointAngularLimitPair &pxlimit);

  INLINE void set_upper_limit(PN_stdfloat angle);
  INLINE PN_stdfloat get_upper_limit() const;

  INLINE void set_lower_limit(PN_stdfloat angle);
  INLINE PN_stdfloat get_lower_limit() const;

  INLINE bool is_valid() const;

public:
  INLINE const physx::PxJointAngularLimitPair &get_limit_pair() const;

protected:
  virtual physx::PxJointLimitParameters &get_params() override;

private:
  physx::PxJointAngularLimitPair _limit;
};

#include "physJointLimitAngularPair.I"

#endif // PHYSJOINTLIMITANGULARPAIR_H
