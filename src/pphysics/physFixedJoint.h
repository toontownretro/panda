/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physFixedJoint.h
 * @author brian
 * @date 2021-04-21
 */

#ifndef PHYSFIXEDJOINT_H
#define PHYSFIXEDJOINT_H

#include "pandabase.h"
#include "physJoint.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysFixedJoint : public PhysJoint {
PUBLISHED:
  PhysFixedJoint(PhysRigidActorNode *a, PhysRigidActorNode *b,
                 const TransformState *frame_a, const TransformState *frame_b);

  virtual ~PhysFixedJoint() override;

  INLINE void set_projection_linear_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_linear_tolerance() const;

  INLINE void set_projection_angular_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_angular_tolerance() const;

public:
  virtual physx::PxJoint *get_joint() const override;

private:
  physx::PxFixedJoint *_joint;
};

#include "physFixedJoint.I"

#endif // PHYSFIXEDJOINT_H
