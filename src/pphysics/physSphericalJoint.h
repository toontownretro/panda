/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physSphericalJoint.h
 * @author brian
 * @date 2021-04-21
 */

#ifndef PHYSSPHERICALJOINT_H
#define PHYSSPHERICALJOINT_H

#include "pandabase.h"
#include "physJoint.h"
#include "physJointLimitCone.h"
#include "deg_2_rad.h"

/**
 * Ball-in-socket type joint.
 */
class EXPCL_PANDA_PPHYSICS PhysSphericalJoint : public PhysJoint {
PUBLISHED:
  PhysSphericalJoint(PhysRigidActorNode *a, PhysRigidActorNode *b,
                     const TransformState *frame_a, const TransformState *frame_b);
  virtual ~PhysSphericalJoint() override;

  INLINE void set_limit_cone(const PhysJointLimitCone &limit);
  INLINE PhysJointLimitCone get_limit_cone() const;
  INLINE bool has_limit_cone() const;
  INLINE void clear_limit_cone();

  INLINE PN_stdfloat get_swing_y_angle() const;
  INLINE PN_stdfloat get_swing_z_angle() const;

  INLINE void set_projection_linear_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_linear_tolerance() const;

public:
  virtual physx::PxJoint *get_joint() const override;

private:
  physx::PxSphericalJoint *_joint;
};

#include "physSphericalJoint.I"

#endif // PHYSSPHERICALJOINT_H
