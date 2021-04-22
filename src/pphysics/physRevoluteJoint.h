/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRevoluteJoint.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSREVOLUTEJOINT_H
#define PHYSREVOLUTEJOINT_H

#include "pandabase.h"
#include "physJoint.h"
#include "physJointLimitAngularPair.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysRevoluteJoint : public PhysJoint {
PUBLISHED:
  PhysRevoluteJoint(PhysRigidActorNode *a, PhysRigidActorNode *b,
                    const TransformState *frame_a, const TransformState *frame_b);
  virtual ~PhysRevoluteJoint() override;

  INLINE PN_stdfloat get_angle() const;
  INLINE PN_stdfloat get_velocity() const;

  INLINE void set_limit(const PhysJointLimitAngularPair &limit);
  INLINE PhysJointLimitAngularPair get_limit() const;
  INLINE bool has_limit() const;
  INLINE void clear_limit();

  INLINE void set_drive(bool flag);
  INLINE bool get_drive() const;

  INLINE void set_drive_freespin(bool flag);
  INLINE bool get_drive_freespin() const;

  INLINE void set_drive_velocity(PN_stdfloat vel, bool autowake = true);
  INLINE PN_stdfloat get_drive_velocity() const;

  INLINE void set_drive_force_limit(PN_stdfloat limit);
  INLINE PN_stdfloat get_drive_force_limit() const;

  INLINE void set_drive_gear_ratio(PN_stdfloat ratio);
  INLINE PN_stdfloat get_drive_gear_ratio() const;

  INLINE void set_projection_linear_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_linear_tolerance() const;

  INLINE void set_projection_angular_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_projection_angular_tolerance() const;

public:
  virtual physx::PxJoint *get_joint() const override;

private:
  physx::PxRevoluteJoint *_joint;
};

#include "physRevoluteJoint.I"

#endif // PHYSREVOLUTEJOINT_H
