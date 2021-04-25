/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physD6Joint.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSD6JOINT_H
#define PHYSD6JOINT_H

#include "pandabase.h"
#include "physJoint.h"
#include "physJointLimitLinearPair.h"
#include "physJointLimitAngularPair.h"
#include "physJointLimitCone.h"
#include "physJointLimitPyramid.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysD6Joint : public PhysJoint {
PUBLISHED:
  enum Motion {
    M_locked,
    M_limited,
    M_free,
  };

  enum Axis {
    A_x,
    A_y,
    A_z,
  };

  PhysD6Joint(PhysRigidActorNode *a, PhysRigidActorNode *b,
              const TransformState *frame_a, const TransformState *frame_b);
  virtual ~PhysD6Joint() override;

  INLINE void set_linear_motion(Axis axis, Motion motion);
  INLINE Motion get_linear_motion(Axis axis) const;

  INLINE void set_angular_motion(Axis axis, Motion motion);
  INLINE Motion get_angular_motion(Axis axis) const;

  INLINE void set_linear_limit(Axis axis, const PhysJointLimitLinearPair &limit);
  INLINE PhysJointLimitLinearPair get_linear_limit(Axis axis) const;

  INLINE void set_twist_limit(const PhysJointLimitAngularPair &limit);
  INLINE PhysJointLimitAngularPair get_twist_limit() const;

  INLINE void set_swing_limit(const PhysJointLimitCone &limit);
  INLINE PhysJointLimitCone get_swing_limit() const;

  INLINE void set_pyramid_swing_limit(const PhysJointLimitPyramid &limit);
  INLINE PhysJointLimitPyramid get_pyramid_swing_limit() const;

  INLINE PN_stdfloat get_angle(Axis axis) const;

public:
  virtual physx::PxJoint *get_joint() const override;

private:
  physx::PxD6Joint *_joint;
};

#include "physD6Joint.I"

#endif // PHYSD6JOINT_H
