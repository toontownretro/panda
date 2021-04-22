/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitLinearPair.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSJOINTLIMITLINEARPAIR_H
#define PHYSJOINTLIMITLINEARPAIR_H

#include "pandabase.h"
#include "physJointLimit.h"
#include "physSystem.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysJointLimitLinearPair : public PhysJointLimit {
PUBLISHED:
  INLINE PhysJointLimitLinearPair(PN_stdfloat lower_limit = -PX_MAX_F32/3.0f,
                                  PN_stdfloat upper_limit = PX_MAX_F32/3.0f,
                                  PN_stdfloat contact_dist = -1.0f);
  INLINE PhysJointLimitLinearPair(PN_stdfloat lower_limit, PN_stdfloat upper_limit,
                                  PN_stdfloat stiffness, PN_stdfloat damping);
  INLINE PhysJointLimitLinearPair(const physx::PxJointLinearLimitPair &pxlimit);

  INLINE void set_upper_limit(PN_stdfloat limit);
  INLINE PN_stdfloat get_upper_limit() const;

  INLINE void set_lower_limit(PN_stdfloat limit);
  INLINE PN_stdfloat get_lower_limit() const;

  INLINE bool is_valid() const;

public:
  INLINE const physx::PxJointLinearLimitPair &get_limit_pair() const;

protected:
  virtual physx::PxJointLimitParameters &get_params() override;

private:
  physx::PxJointLinearLimitPair _limit;
};

#include "physJointLimitLinearPair.I"

#endif // PHYSJOINTLIMITLINEARPAIR_H
