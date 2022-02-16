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
#include "physContactCallbackData.h"
#include "physEnums.h"
#include "randomizer.h"
#include "config_pphysics.h"
#include "clockObject.h"

/**
 *
 */
PhysRagdoll::
PhysRagdoll(const NodePath &character_np) {
  _char_np = character_np;
  _char_node = DCAST(CharacterNode, _char_np.find("**/+CharacterNode").node());
  _char = _char_node->get_character();
  _enabled = false;
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

  _contact_callback = new LimbContactCallback(this);
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
          PN_stdfloat damping,
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
    joint->actor->set_transform(joint_pose);
    joint->actor->set_contact_callback(_contact_callback);
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

      } else if (joint->limit_x[0] == -180 && joint->limit_x[1] == 180) {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_free);

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_limited);
        djoint->set_twist_limit(
          PhysJointLimitAngularPair(joint->limit_x[0], joint->limit_x[1],
            (joint->limit_x[1] - joint->limit_x[0]) * phys_ragdoll_contact_distance_ratio));
      }

      bool y_locked = false;
      bool z_locked = false;

      if (joint->limit_y[0] == 0 && joint->limit_y[1] == 0) {
        djoint->set_angular_motion(PhysD6Joint::A_y, PhysD6Joint::M_locked);
        y_locked = true;

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_y, PhysD6Joint::M_limited);
      }

      if (joint->limit_z[0] == 0 && joint->limit_z[1] == 0) {
        djoint->set_angular_motion(PhysD6Joint::A_z, PhysD6Joint::M_locked);
        z_locked = true;

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_z, PhysD6Joint::M_limited);
      }

      if (!y_locked || !z_locked) {
        PN_stdfloat dist_y = (joint->limit_y[1] - joint->limit_y[0]) * phys_ragdoll_contact_distance_ratio;
        PN_stdfloat dist_z = (joint->limit_z[1] - joint->limit_z[0]) * phys_ragdoll_contact_distance_ratio;
        djoint->set_pyramid_swing_limit(
          PhysJointLimitPyramid(joint->limit_y[0], joint->limit_y[1], joint->limit_z[0], joint->limit_z[1], std::max(dist_y, dist_z)));
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
  create_joints();

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
  _enabled = true;
}

/**
 *
 */
void PhysRagdoll::
stop_ragdoll() {
  if (_aggregate->getScene() != nullptr) {
    _aggregate->getScene()->removeAggregate(*_aggregate);
  }
  _enabled = false;
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
      _aggregate->getScene()->removeAggregate(*_aggregate);
    }
  }

  clear_joints();

  if (_aggregate != nullptr) {
    _aggregate->release();
    _aggregate = nullptr;
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
void PhysRagdoll::
update() {
  if (!_enabled) {
    return;
  }

  CPT(TransformState) char_net = _char_np.get_net_transform();

  for (size_t i = 0; i < _char_joints.size(); i++) {
    Joint *limb = _char_joints[i];

    if (limb == nullptr || limb->actor == nullptr) {
      _char->set_joint_forced_value(i, _char->get_joint_default_value(i));
      continue;
    }

    CPT(TransformState) limb_actor_transform = limb->actor->get_transform();
    if (limb_actor_transform == nullptr) {
      _char->set_joint_forced_value(i, _char->get_joint_default_value(i));
      continue;
    }

    LMatrix4 net_inverse = LMatrix4::ident_mat();
    int parent = _char->get_joint_parent(limb->joint);
    if (parent != -1) {
      net_inverse = _char->get_joint_net_transform(parent);
    } else {
      net_inverse = _char->get_root_xform();
    }
    net_inverse.invert_in_place();
    CPT(TransformState) joint_trans_state = char_net->
      invert_compose(limb_actor_transform);
    LMatrix4 joint_trans = joint_trans_state->get_mat();
    _char->set_joint_forced_value(limb->joint, joint_trans * net_inverse);

    if (_debug) {
      limb->debug.set_transform(limb->actor->get_transform());
      limb->debug.set_scale(_debug_scale);
    }
  }
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
void PhysRagdoll::LimbContactCallback::
do_callback(CallbackData *cbdata) {
  PhysContactCallbackData *data = (PhysContactCallbackData *)cbdata;

  if (_ragdoll.was_deleted()) {
    return;
  }

  if (_ragdoll->_hard_impact_sounds.empty() && _ragdoll->_soft_impact_sounds.empty()) {
    return;
  }

  if (data->get_num_contact_pairs() == 0) {
    return;
  }

  const PhysContactPair *pair = data->get_contact_pair(0);
  if (!pair->is_contact_type(PhysEnums::CT_found)) {
    return;
  }
  if (pair->get_num_contact_points() == 0) {
    return;
  }
  PhysContactPoint point = pair->get_contact_point(0);

  ClockObject *clock = ClockObject::get_global_clock();
  double dt = clock->get_dt();

  if (dt > 0.1) {
    return;
  }

  PN_stdfloat speed = point.get_impulse().length();
  if (speed < 70.0f) {
    return;
  }

  LPoint3 position = point.get_position();

  //std::cout << "Ragdoll contact between " << NodePath(data->get_actor_a()) << " and " << NodePath(data->get_actor_b()) << " force " << force_magnitude << "\n";

  Randomizer random;

  float volume = speed * speed * (1.0f / (320.0f * 320.0f));
  volume = std::min(1.0f, volume);

  if (speed >= _ragdoll->_hard_impact_force && !_ragdoll->_hard_impact_sounds.empty()) {
    int index = random.random_int(_ragdoll->_hard_impact_sounds.size());
    _ragdoll->_hard_impact_sounds[index]->set_volume(volume);
    _ragdoll->_hard_impact_sounds[index]->set_3d_attributes(
      position[0], position[1], position[2], 0.0, 0.0, 0.0);
    _ragdoll->_hard_impact_sounds[index]->play();
  } else if (speed >= _ragdoll->_soft_impact_force && !_ragdoll->_soft_impact_sounds.empty()) {
    int index = random.random_int(_ragdoll->_soft_impact_sounds.size());
    _ragdoll->_soft_impact_sounds[index]->set_volume(volume);
    _ragdoll->_soft_impact_sounds[index]->set_3d_attributes(
      position[0], position[1], position[2], 0.0, 0.0, 0.0);
    _ragdoll->_soft_impact_sounds[index]->play();
  }
}
