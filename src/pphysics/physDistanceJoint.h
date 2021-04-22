/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physDistanceJoint.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSDISTANCEJOINT_H
#define PHYSDISTANCEJOINT_H

#include "pandabase.h"
#include "physJoint.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysDistanceJoint : public PhysJoint {
PUBLISHED:
  PhysDistanceJoint(PhysRigidActorNode *a, PhysRigidActorNode *b,
                    const TransformState *frame_a, const TransformState *frame_b);
  virtual ~PhysDistanceJoint() override;

  INLINE PN_stdfloat get_distance() const;

  INLINE void set_min_distance(PN_stdfloat distance);
  INLINE PN_stdfloat get_min_distance() const;
  INLINE bool has_min_distance() const;
  INLINE void clear_min_distance();

  INLINE void set_max_distance(PN_stdfloat distance);
  INLINE PN_stdfloat get_max_distance() const;
  INLINE bool has_max_distance() const;
  INLINE void clear_max_distance();

  INLINE void set_tolerance(PN_stdfloat tolerance);
  INLINE PN_stdfloat get_tolerance() const;

  INLINE void set_spring(bool flag);
  INLINE bool get_spring() const;

  INLINE void set_stiffness(PN_stdfloat stiffness);
  INLINE PN_stdfloat get_stiffness() const;

  INLINE void set_damping(PN_stdfloat damping);
  INLINE PN_stdfloat get_damping() const;

public:
  virtual physx::PxJoint *get_joint() const override;

private:
  physx::PxDistanceJoint *_joint;
};

#include "physDistanceJoint.I"

#endif // PHYSDISTANCEJOINT_H
