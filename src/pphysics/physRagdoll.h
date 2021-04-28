/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRagdoll.h
 * @author brian
 * @date 2021-04-22
 */

#ifndef PHYSRAGDOLL_H
#define PHYSRAGDOLL_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "physRigidDynamicNode.h"
#include "physShape.h"
#include "physD6Joint.h"
#include "nodePath.h"
#include "characterNode.h"
#include "character.h"
#include "pmap.h"
#include "physScene.h"
#include "audioSound.h"

#include "physx_includes.h"

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysRagdoll : public ReferenceCount {
PUBLISHED:
  class EXPCL_PANDA_PPHYSICS LimbContactCallback : public CallbackObject {
  public:
    ALLOC_DELETED_CHAIN(LimbContactCallback);

    INLINE LimbContactCallback(PhysRagdoll *ragdoll) { _ragdoll = ragdoll; }

    virtual void do_callback(CallbackData *cbdata) override;

  private:
    PhysRagdoll *_ragdoll;
  };

  class Joint : public ReferenceCount {
  public:
    Joint *parent;
    int joint;

    // Computed automatically:
    PN_stdfloat mass;
    PN_stdfloat volume;
    PN_stdfloat surface_area;

    PN_stdfloat damping;
    PN_stdfloat angular_damping;
    PN_stdfloat inertia;
    PN_stdfloat mass_bias;
    PN_stdfloat thickness;
    PN_stdfloat density;

    LVecBase2 limit_x;
    LVecBase2 limit_y;
    LVecBase2 limit_z;

    PT(PhysShape) shape;
    PT(PhysRigidDynamicNode) actor;
    PT(PhysD6Joint) djoint;

    NodePath debug;
  };

  PhysRagdoll(const NodePath &character_np);
  ~PhysRagdoll();

  void add_joint(const std::string &parent, const std::string &child,
                 PhysShape *shape, PN_stdfloat mass_bias, PN_stdfloat rot_damping, PN_stdfloat density,
                 PN_stdfloat damping, PN_stdfloat thickness, PN_stdfloat inertia,
                 const LVecBase2 &limit_x, const LVecBase2 &limit_y, const LVecBase2 &limit_z);

  void compute_mass();

  void start_ragdoll(PhysScene *scene, NodePath render);
  void stop_ragdoll();

  PhysRigidDynamicNode *get_joint_actor(const std::string &name) const;
  PhysD6Joint *get_joint_constraint(const std::string &name) const;

  void update();

  void create_joints();
  void clear_joints();

  void destroy();

  void set_total_mass(PN_stdfloat mass);
  void set_debug(bool flag, PN_stdfloat scale = 1.0f);

  void set_impact_forces(PN_stdfloat soft, PN_stdfloat hard);

  void add_hard_impact_sound(AudioSound *blah);

  void add_soft_impact_sound(AudioSound *blah);

private:
  CPT(TransformState) joint_default_net_transform(int joint);

private:

  PT(LimbContactCallback) _contact_callback;

  typedef pvector<PT(AudioSound)> Sounds;
  Sounds _hard_impact_sounds;
  Sounds _soft_impact_sounds;

  PN_stdfloat _hard_impact_force;
  PN_stdfloat _soft_impact_force;

  physx::PxAggregate *_aggregate;

  // Mass of ragdoll.  The code will distribute this mass to each part based
  // on the collision model's volume.
  PN_stdfloat _total_mass;
  PN_stdfloat _total_volume;

  bool _enabled;

  bool _debug;
  PN_stdfloat _debug_scale;

  typedef pmap<std::string, Joint *> Joints;
  Joints _joints;
  typedef pvector<PT(Joint)> AllJoints;
  AllJoints _all_joints;

  typedef pvector<Joint *> JointStars;
  JointStars _char_joints;

  NodePath _char_np;
  CharacterNode *_char_node;
  Character *_char;

  friend class LimbContactCallback;
};

#include "physRagdoll.I"

#endif // PHYSRAGDOLL_H
