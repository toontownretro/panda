/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physPrismaticJoint.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSPRISMATICJOINT_H
#define PHYSPRISMATICJOINT_H

#include "pandabase.h"
#include "physJoint.h"
#include "physJointLimitLinearPair.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysPrismaticJoint : public PhysJoint {
PUBLISHED:
  PhysPrismaticJoint(PhysRigidActorNode *a, PhysRigidActorNode *b,
                     const TransformState *frame_a, const TransformState *frame_b);
  virtual ~PhysPrismaticJoint() override;

  INLINE PN_stdfloat get_position() const;
  INLINE PN_stdfloat get_velocity() const;

  INLINE void set_limit(const PhysJointLimitLinearPair &limit);
  INLINE PhysJointLimitLinearPair get_limit() const;
  INLINE bool has_limit() const;
  INLINE void clear_limit();

  INLINE void set_projection_linear_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_linear_tolerance() const;

  INLINE void set_projection_angular_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_angular_tolerance() const;

public:
  virtual physx::PxJoint *get_joint() const override;

private:
  physx::PxPrismaticJoint *_joint;
};

#include "physPrismaticJoint.I"

#endif // PHYSPRISMATICJOINT_H
