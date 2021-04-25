/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJointLimitPyramid.h
 * @author brian
 * @date 2021-04-23
 */

#ifndef PHYSJOINTLIMITPYRAMID_H
#define PHYSJOINTLIMITPYRAMID_H

#include "pandabase.h"
#include "physJointLimit.h"
#include "deg_2_rad.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysJointLimitPyramid : public PhysJointLimit {
PUBLISHED:
  INLINE PhysJointLimitPyramid(PN_stdfloat y_min, PN_stdfloat y_max,
                               PN_stdfloat z_min, PN_stdfloat z_max,
                               PN_stdfloat contact_dist = -1.0f);
  INLINE PhysJointLimitPyramid(PN_stdfloat y_min, PN_stdfloat y_max,
                               PN_stdfloat z_min, PN_stdfloat z_max,
                               PN_stdfloat stiffness, PN_stdfloat damping);
  INLINE PhysJointLimitPyramid(const physx::PxJointLimitPyramid &pxlimit);

  INLINE void set_y_range(PN_stdfloat y_min, PN_stdfloat y_max);
  INLINE PN_stdfloat get_y_min() const;
  INLINE PN_stdfloat get_y_max() const;

  INLINE void set_z_range(PN_stdfloat z_min, PN_stdfloat z_max);
  INLINE PN_stdfloat get_z_min() const;
  INLINE PN_stdfloat get_z_max() const;

  INLINE bool is_valid() const;

public:
  INLINE const physx::PxJointLimitPyramid &get_limit_pyramid() const;

protected:
  virtual physx::PxJointLimitParameters &get_params() override;

private:
  physx::PxJointLimitPyramid _limit;
};

#include "physJointLimitPyramid.I"

#endif // PHYSJOINTLIMITPYRAMID_H
