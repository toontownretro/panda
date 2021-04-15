/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRigidBodyNode.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSRIGIDBODYNODE_H
#define PHYSRIGIDBODYNODE_H

#include "pandabase.h"
#include "physRigidActorNode.h"

/**
 * Base class for PhysRigidDynamicNode.  A rigid body with mass and velocity.
 */
class EXPCL_PANDA_PPHYSICS PhysRigidBodyNode : public PhysRigidActorNode {
PUBLISHED:
  INLINE void set_mass(PN_stdfloat mass);
  INLINE PN_stdfloat get_mass() const;

  INLINE PN_stdfloat get_inv_mass() const;

  INLINE void set_linear_damping(PN_stdfloat damping);
  INLINE PN_stdfloat get_linear_damping() const;

  INLINE void set_angular_damping(PN_stdfloat damping);
  INLINE PN_stdfloat get_angular_damping() const;

  INLINE void set_linear_velocity(const LVector3 &vel, bool auto_wake = true);
  INLINE LVector3 get_linear_velocity() const;

  INLINE void set_max_linear_velocity(PN_stdfloat max);
  INLINE PN_stdfloat get_max_linear_velocity() const;

  INLINE void set_angular_velocity(const LVector3 &vel, bool auto_wake = true);
  INLINE LVector3 get_angular_velocity() const;

  INLINE void set_max_angular_velocity(PN_stdfloat max);
  INLINE PN_stdfloat get_max_angular_velocity() const;

  INLINE void add_force(const LVector3 &force, bool auto_wake = true);
  INLINE void clear_force();

  INLINE void add_impulse_force(const LVector3 &force, bool auto_wake = true);
  INLINE void clear_impulse_force();

  INLINE void add_torque(const LVector3 &torque, bool auto_wake = true);
  INLINE void clear_torque();

  INLINE void add_impulse_torque(const LVector3 &torque, bool auto_wake = true);
  INLINE void clear_impulse_torque();

  INLINE void set_min_ccd_advance_coefficient(PN_stdfloat coef);
  INLINE PN_stdfloat get_min_ccd_advance_coefficient() const;

  INLINE void set_max_depenetration_velocity(PN_stdfloat bias_clamp);
  INLINE PN_stdfloat get_max_depenetration_velocity() const;

  INLINE void set_max_contact_impulse(PN_stdfloat max);
  INLINE PN_stdfloat get_max_contact_impulse() const;

  INLINE void set_ccd_enabled(bool flag);
  INLINE bool get_ccd_enabled() const;

  INLINE void set_retain_accelerations(bool flag);
  INLINE bool get_retain_accelerations() const;

  INLINE void set_kinematic(bool flag);
  INLINE bool is_kinematic() const;

protected:
  PhysRigidBodyNode(const std::string &name);

public:
  virtual physx::PxRigidActor *get_rigid_actor() const override;
  virtual physx::PxRigidBody *get_rigid_body() const = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysRigidActorNode::init_type();
    register_type(_type_handle, "PhysRigidBodyNode",
                  PhysRigidActorNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physRigidBodyNode.I"

#endif // PHYSRIGIDBODYNODE_H
