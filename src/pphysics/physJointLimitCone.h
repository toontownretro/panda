/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitCone.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSJOINTLIMITCONE_H
#define PHYSJOINTLIMITCONE_H

#include "pandabase.h"
#include "physJointLimit.h"
#include "deg_2_rad.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysJointLimitCone : public PhysJointLimit {
PUBLISHED:
  PhysJointLimitCone(PN_stdfloat y_limit, PN_stdfloat z_limit,
                     PN_stdfloat contact_distance = -1.0f);

  INLINE PhysJointLimitCone(const physx::PxJointLimitCone &pxlimit);

  INLINE void set_y_limit_angle(PN_stdfloat angle);
  INLINE PN_stdfloat get_y_limit_angle() const;

  INLINE void set_z_limit_angle(PN_stdfloat angle);
  INLINE PN_stdfloat get_z_limit_angle() const;

  INLINE bool is_valid() const;

public:
  INLINE const physx::PxJointLimitCone &get_limit_cone() const;

protected:
  virtual physx::PxJointLimitParameters &get_params() override;

private:
  physx::PxJointLimitCone _limit;
};

#include "physJointLimitCone.I"

#endif // PHYSJOINTLIMITCONE_H
