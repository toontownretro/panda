/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physRagdoll.cxx
 * @author brian
 * @date 2021-04-22
 */

#include "physRagdoll.h"
#include "physSystem.h"
#include "look_at.h"
#include "loader.h"
#include "physSleepStateCallbackData.h"
#include "physEnums.h"
#include "randomizer.h"
#include "config_pphysics.h"
#include "clockObject.h"
#include "jobSystem.h"
#include "boundingBox.h"

pvector<PT(PhysRagdoll)> PhysRagdoll::_all_ragdolls;

static ConfigVariableDouble phys_ragdoll_joint_stiffness("phys-ragdoll-joint-stiffness", 0.0);
static ConfigVariableDouble phys_ragdoll_joint_damping("phys-ragdoll-joint-damping", 0.0);
static ConfigVariableDouble phys_ragdoll_joint_restitution("phys-ragdoll-joint-restitution", 0.0);
static ConfigVariableDouble phys_ragdoll_joint_bounce_threshold("phys-ragdoll-joint-bounce-threshold", 0.0);

/**
 *
 */
PhysRagdoll::
PhysRagdoll(const NodePath &character_np) {
  _char_np = character_np;
  _char_node = DCAST(CharacterNode, _char_np.find("**/+CharacterNode").node());
  _char = _char_node->get_character();
  _enabled = false;
  _awake_joints = 0;
  PhysSystem *sys = PhysSystem::ptr();
  physx::PxPhysics *physics = sys->get_physics();
  _aggregate = physics->createAggregate(_char->get_num_joints(), true);

  _soft_impact_force = 100;
  _hard_impact_force = 500;

  _debug = false;
  _debug_scale = 1.0f;

  _char_joints.resize(_char->get_num_joints());
  for (size_t i = 0; i < _char_joints.size(); i++) {
    _char_joints[i] = nullptr;
  }

  _sleep_callback = new LimbSleepCallback(this);
}

/**
 *
 */
PhysRagdoll::
~PhysRagdoll() {
  destroy();
}

/**
 *
 */
CPT(TransformState) PhysRagdoll::
joint_default_net_transform(int joint) {
  LMatrix4 initial_net = _char->get_joint_initial_net_transform_inverse(joint);
  initial_net.invert_in_place();
  CPT(TransformState) joint_trans = TransformState::make_mat(initial_net * _char_np.get_net_transform()->get_mat());
  return joint_trans;
}

/**
 *
 */
void PhysRagdoll::
add_joint(const std::string &parent, const std::string &child,
          PhysShape *shape, PN_stdfloat mass, PN_stdfloat rot_damping,
          PN_stdfloat damping, PN_stdfloat inertia,
          const LVecBase2 &limit_x, const LVecBase2 &limit_y, const LVecBase2 &limit_z) {

  PT(Joint) joint = new Joint;
  if (!parent.empty()) {
    joint->parent = _joints[parent];
  } else {
    joint->parent = nullptr;
  }
  joint->joint = _char->find_joint(child);

  joint->mass = mass;
  joint->damping = damping;
  joint->angular_damping = rot_damping;
  joint->inertia = inertia;

  joint->limit_x = limit_x;
  joint->limit_y = limit_y;
  joint->limit_z = limit_z;

  joint->shape = shape;
  joint->actor = nullptr;
  joint->djoint = nullptr;

  if (_debug) {
    joint->debug = NodePath(Loader::get_global_ptr()->load_sync("models/misc/smiley.bam"));
    joint->debug.set_render_mode_wireframe();
    joint->debug.set_scale(_debug_scale);
  }

  _joints[child] = joint;
  _all_joints.push_back(joint);
  _char_joints[joint->joint] = joint;
}

/**
 *
 */
void PhysRagdoll::
create_joints() {
  for (size_t i = 0; i < _all_joints.size(); i++) {
    Joint *joint = _all_joints[i];

    CPT(TransformState) joint_pose = joint_default_net_transform(joint->joint);

    joint->actor = new PhysRigidDynamicNode(_char->get_joint_name(joint->joint));
    joint->actor->set_ccd_enabled(true);
    joint->actor->add_shape(joint->shape);
    joint->actor->set_mass(joint->mass);
    joint->actor->set_angular_damping(joint->angular_damping);
    joint->actor->set_linear_damping(joint->damping);
    joint->actor->set_inertia_tensor(joint->actor->get_inertia_tensor() * (joint->inertia * 0.5f));
    joint->actor->set_transform(joint_pose);
    joint->actor->set_sleep_callback(_sleep_callback);
    joint->actor->set_wake_callback(_sleep_callback);
    joint->actor->set_sleep_threshold(0.25f);
    //joint->actor->set_contact_callback(_contact_callback);
    joint->actor->set_max_depenetration_velocity(phys_ragdoll_max_depenetration_vel);
    joint->actor->set_num_position_iterations(phys_ragdoll_pos_iterations);
    joint->actor->set_num_velocity_iterations(phys_ragdoll_vel_iterations);

    if (joint->parent != nullptr) {
      CPT(TransformState) parent_pose = joint_default_net_transform(joint->parent->joint)->invert_compose(joint_pose);

      PT(PhysD6Joint) djoint = new PhysD6Joint(joint->parent->actor, joint->actor,
                                              parent_pose, TransformState::make_identity());
      djoint->set_linear_motion(PhysD6Joint::A_x, PhysD6Joint::M_locked);
      djoint->set_linear_motion(PhysD6Joint::A_y, PhysD6Joint::M_locked);
      djoint->set_linear_motion(PhysD6Joint::A_z, PhysD6Joint::M_locked);
      djoint->set_projection_enabled(phys_ragdoll_projection);
      if (phys_ragdoll_projection) {
        djoint->set_projection_angular_tolerance(phys_ragdoll_projection_angular_tolerance);
        djoint->set_projection_linear_tolerance(phys_ragdoll_projection_linear_tolerance);
      }
      djoint->set_collision_enabled(false);

      if (joint->limit_x[0] == 0 && joint->limit_x[1] == 0) {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_locked);

      } else if (joint->limit_x[0] <= -180 && joint->limit_x[1] >= 180) {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_free);

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_limited);
        PhysJointLimitAngularPair limit(joint->limit_x[0], joint->limit_x[1],
            (joint->limit_x[1] - joint->limit_x[0]) * phys_ragdoll_contact_distance_ratio);
        limit.set_stiffness(phys_ragdoll_joint_stiffness);
        limit.set_damping(phys_ragdoll_joint_damping);
        limit.set_restitution(phys_ragdoll_joint_restitution);
        limit.set_bounce_threshold(phys_ragdoll_joint_bounce_threshold);
        djoint->set_twist_limit(limit);
      }

      bool y_locked_or_free = false;
      bool z_locked_or_free = false;

      if (joint->limit_y[0] == 0 && joint->limit_y[1] == 0) {
        djoint->set_angular_motion(PhysD6Joint::A_y, PhysD6Joint::M_locked);
        y_locked_or_free = true;

      } else if (joint->limit_y[0] <= -180 && joint->limit_y[1] >= 180) {
        djoint->set_angular_motion(PhysD6Joint::A_y, PhysD6Joint::M_free);
        y_locked_or_free = true;

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_y, PhysD6Joint::M_limited);
      }

      if (joint->limit_z[0] == 0 && joint->limit_z[1] == 0) {
        djoint->set_angular_motion(PhysD6Joint::A_z, PhysD6Joint::M_locked);
        z_locked_or_free = true;

      } else if (joint->limit_z[0] <= -180 && joint->limit_z[1] >= 180) {
        djoint->set_angular_motion(PhysD6Joint::A_z, PhysD6Joint::M_free);
        z_locked_or_free = true;

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_z, PhysD6Joint::M_limited);
      }

      if (!y_locked_or_free || !z_locked_or_free) {
        PN_stdfloat dist_y = 0.0f;
        PN_stdfloat dist_z = 0.0f;
        if (!y_locked_or_free) {
          dist_y = (joint->limit_y[1] - joint->limit_y[0]) * phys_ragdoll_contact_distance_ratio;
        } else {
          dist_z = (joint->limit_z[1] - joint->limit_z[0]) * phys_ragdoll_contact_distance_ratio;
        }
        PhysJointLimitPyramid limit(joint->limit_y[0], joint->limit_y[1], joint->limit_z[0], joint->limit_z[1], std::max(dist_y, dist_z));
        limit.set_stiffness(phys_ragdoll_joint_stiffness);
        limit.set_damping(phys_ragdoll_joint_damping);
        limit.set_restitution(phys_ragdoll_joint_restitution);
        limit.set_bounce_threshold(phys_ragdoll_joint_bounce_threshold);
        djoint->set_pyramid_swing_limit(limit);
      }

      joint->djoint = djoint;
    }

    _aggregate->addActor(*joint->actor->get_rigid_actor());
  }
}

/**
 *
 */
void PhysRagdoll::
start_ragdoll(PhysScene *scene, NodePath render) {
  if (_enabled) {
    return;
  }

  create_joints();

  _awake_joints = 0;

  for (size_t i = 0; i < _char_joints.size(); i++) {
    Joint *joint = _char_joints[i];
    if (joint == nullptr) {
      // Not ragdolled, force to bind/rest pose.
      _char->set_joint_forced_value(i, _char->get_joint_default_value(i));

    } else {
      // It's a ragdolled joint, set the actor to the current joint's pose.

      joint->actor->set_transform(
      TransformState::make_mat(
        _char->get_joint_net_transform(joint->joint) * _char_np.get_net_transform()->get_mat()));
      if (_debug) {
        joint->debug.reparent_to(render);
        joint->debug.set_transform(joint->actor->get_transform());
      }
    }
  }

  scene->get_scene()->addAggregate(*_aggregate);
  for (const Joint *joint : _all_joints) {
    scene->add_actor(joint->actor);
  }
  _enabled = true;

  _all_ragdolls.push_back(this);
}

/**
 *
 */
void PhysRagdoll::
stop_ragdoll() {
  if (!_enabled) {
    return;
  }

  if (_aggregate->getScene() != nullptr) {
    physx::PxScene *scene = _aggregate->getScene();
    PhysScene *ps = (PhysScene *)scene->userData;
    scene->removeAggregate(*_aggregate);

    for (const Joint *joint : _all_joints) {
      ps->remove_actor(joint->actor);
    }
  }
  _enabled = false;

  auto it = std::find(_all_ragdolls.begin(), _all_ragdolls.end(), this);
  if (it != _all_ragdolls.end()) {
    _all_ragdolls.erase(it);
  }
}

/**
 * Returns the rigid body node corresponding to the given character joint.
 */
PhysRigidDynamicNode *PhysRagdoll::
get_joint_actor(const std::string &name) const {
  Joints::const_iterator it = _joints.find(name);
  if (it == _joints.end()) {
    return nullptr;
  }

  return (*it).second->actor;
}

/**
 * Returns the rigid body node corresponding to the given character joint.
 */
PhysRigidDynamicNode *PhysRagdoll::
get_joint_actor(int n) const {
  nassertr(n >= 0 && n < (int)_all_joints.size(), nullptr);
  return _all_joints[n]->actor;
}

/**
 * Returns the constraint between the given character joint and its parent.
 */
PhysD6Joint *PhysRagdoll::
get_joint_constraint(const std::string &name) const {
  Joints::const_iterator it = _joints.find(name);
  if (it == _joints.end()) {
    return nullptr;
  }

  return (*it).second->djoint;
}

/**
 *
 */
void PhysRagdoll::
clear_joints() {
  if (_aggregate != nullptr) {
    for (size_t i = 0; i < _all_joints.size(); i++) {
      _aggregate->removeActor(*(_all_joints[i]->actor->get_rigid_actor()));
    }
  }

  _joints.clear();
  _all_joints.clear();
  _char_joints.clear();
}

/**
 *
 */
void PhysRagdoll::
destroy() {
  if (_aggregate != nullptr) {
    if (_aggregate->getScene() != nullptr) {
      physx::PxScene *scene = _aggregate->getScene();
      PhysScene *ps = (PhysScene *)scene->userData;
      scene->removeAggregate(*_aggregate);

      for (const Joint *joint : _all_joints) {
        ps->remove_actor(joint->actor);
      }
    }
  }

  clear_joints();

  if (_aggregate != nullptr) {
    _aggregate->release();
    _aggregate = nullptr;
  }

  if (_enabled) {
    auto it = std::find(_all_ragdolls.begin(), _all_ragdolls.end(), this);
    if (it != _all_ragdolls.end()) {
      _all_ragdolls.erase(it);
    }
  }

  _enabled = false;
}

/**
 *
 */
void PhysRagdoll::
set_debug(bool flag, PN_stdfloat scale) {
  _debug = flag;
  _debug_scale = scale;
}

/**
 * Returns the number of ragdoll joints.
 */
int PhysRagdoll::
get_num_joints() const {
  return (int)_all_joints.size();
}

/**
 * Returns the ragdoll joint with the indicated name, or nullptr if no such
 * joint exists.
 */
PhysRagdoll::Joint *PhysRagdoll::
get_joint_by_name(const std::string &name) const {
  auto it = _joints.find(name);
  if (it != _joints.end()) {
    return (*it).second;
  }
  return nullptr;
}

/**
 * Returns the nth ragdoll joint.
 */
PhysRagdoll::Joint *PhysRagdoll::
get_joint(int n) const {
  nassertr(n >= 0 && n < (int)_all_joints.size(), nullptr);
  return _all_joints[n];
}

/**
 *
 */
bool PhysRagdoll::
update() {
  if (!_enabled) {
    return false;
  }

  if (_awake_joints <= 0) {
    return false;
  }

  CPT(TransformState) char_net = _char_np.get_net_transform();
  LMatrix4 world_to_char;
  world_to_char.invert_from(char_net->get_mat());

  LMatrix4 char_root_to_parent;
  LMatrix4 local_trans;

  LPoint3 mins(999999);
  LPoint3 maxs(-999999);

  for (size_t i = 0; i < _char_joints.size(); i++) {
    Joint *limb = _char_joints[i];

    if (limb == nullptr || limb->actor == nullptr) {
      continue;
    }

    //if (limb == nullptr || limb->actor == nullptr) {
    //  _char->set_joint_forced_value(i, _char->get_joint_default_value(i));
    //  continue;
    //}

    CPT(TransformState) limb_actor_transform = limb->actor->get_transform();
    const LMatrix4 &limb_actor_mat = limb_actor_transform->get_mat();
    //if (limb_actor_transform == nullptr) {
    //  _char->set_joint_forced_value(i, _char->get_joint_default_value(i));
    //  continue;
    //}

    int parent = _char->get_joint_parent(limb->joint);
    if (parent != -1) {
      char_root_to_parent.invert_from(_char->get_joint_net_transform(parent));
    } else {
      char_root_to_parent.invert_from(_char->get_root_xform());
    }

    // First move world-space limb actor transform into character-root space.
    local_trans = limb_actor_mat * world_to_char;
    // Then move into the coordinate space of the parent joint of this ragdoll
    // joint.
    local_trans *= char_root_to_parent;
    _char->set_joint_forced_value(limb->joint, local_trans);

    if (_debug) {
      limb->debug.set_transform(limb->actor->get_transform());
      limb->debug.set_scale(_debug_scale);
    }
  }

  // Now update the bounding box of the ragdoll based on the joint positions.
  for (int i = 0; i < (int)_all_joints.size(); ++i) {
    PhysRigidActorNode *actor = _all_joints[i]->actor;
    physx::PxBounds3 px_bounds = actor->get_rigid_actor()->getWorldBounds();
    mins = mins.fmin(physx_vec_to_panda(px_bounds.minimum));
    maxs = maxs.fmax(physx_vec_to_panda(px_bounds.maximum));
  }

  // Transform bbox to be relative to the character node.
  mins = world_to_char.xform_point(mins);
  maxs = world_to_char.xform_point(maxs);

  _char_node->set_bounds(new BoundingBox(mins, maxs));

  return true;
}

/**
 *
 */
void PhysRagdoll::
set_impact_forces(PN_stdfloat soft, PN_stdfloat hard) {
  _soft_impact_force = soft;
  _hard_impact_force = hard;
}

/**
 *
 */
void PhysRagdoll::
add_hard_impact_sound(AudioSound *sound) {
  _hard_impact_sounds.push_back(sound);
}

/**
 *
 */
void PhysRagdoll::
add_soft_impact_sound(AudioSound *sound) {
  _soft_impact_sounds.push_back(sound);
}

/**
 *
 */
void PhysRagdoll::
update_ragdolls() {
  JobSystem *jsys = JobSystem::get_global_ptr();
  jsys->parallel_process(_all_ragdolls.size(), [&](int i) {
    PhysRagdoll *ragdoll = _all_ragdolls[i];
    ragdoll->update();
  });
}

/**
 *
 */
void PhysRagdoll::LimbSleepCallback::
do_callback(CallbackData *cbdata) {
  PhysSleepStateCallbackData *data = (PhysSleepStateCallbackData *)cbdata;

  if (!_ragdoll.is_valid_pointer()) {
    return;
  }

  if (data->is_asleep()) {
    --_ragdoll->_awake_joints;
  } else {
    ++_ragdoll->_awake_joints;
  }
}
