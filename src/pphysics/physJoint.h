/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physJoint.h
 * @author brian
 * @date 2021-04-21
 */

#ifndef PHYSJOINT_H
#define PHYSJOINT_H

#include "pandabase.h"
#include "referenceCount.h"
#include "physRigidActorNode.h"
#include "callbackObject.h"

#include "physx_includes.h"
#include "physx_utils.h"

class EXPCL_PANDA_PPHYSICS PhysJoint : public ReferenceCount {
protected:
  PhysJoint() = default;

PUBLISHED:
  virtual ~PhysJoint() = default;

  INLINE void set_actors(PhysRigidActorNode *a, PhysRigidActorNode *b);
  INLINE void get_actors(PhysRigidActorNode *&a, PhysRigidActorNode *&b) const;

  INLINE void set_frame_a(const TransformState *transform);
  INLINE CPT(TransformState) get_frame_a() const;

  INLINE void set_frame_b(const TransformState *transform);
  INLINE CPT(TransformState) get_frame_b() const;

  INLINE CPT(TransformState) get_relative_transform() const;

  INLINE LVector3 get_relative_linear_velocity() const;
  INLINE LVector3 get_relative_angular_velocity() const;

  INLINE void set_break_force(PN_stdfloat force, PN_stdfloat torque);
  INLINE void get_break_force(PN_stdfloat &force, PN_stdfloat &torque) const;

  INLINE void set_inv_mass_scale_a(PN_stdfloat scale);
  INLINE PN_stdfloat get_inv_mass_scale_a() const;

  INLINE void set_inv_inertia_scale_a(PN_stdfloat scale);
  INLINE PN_stdfloat get_inv_inertia_scale_a() const;

  INLINE void set_inv_mass_scale_b(PN_stdfloat scale);
  INLINE PN_stdfloat get_inv_mass_scale_b() const;

  INLINE void set_inv_inertia_scale_b(PN_stdfloat scale);
  INLINE PN_stdfloat get_inv_inertia_scale_b() const;

  INLINE void set_projection_enabled(bool flag);
  INLINE bool get_projection_enabled() const;

  INLINE void set_collision_enabled(bool flag);
  INLINE bool get_collision_enabled() const;

  INLINE bool is_broken() const;

  INLINE void set_break_callback(CallbackObject *callback);
  INLINE CallbackObject *get_break_callback() const;

private:
  PT(CallbackObject) _break_callback;

public:
  virtual physx::PxJoint *get_joint() const = 0;
};

#include "physJoint.I"

#endif // PHYSJOINT_H
