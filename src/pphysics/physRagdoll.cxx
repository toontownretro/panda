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

  _total_mass = 0.0f;
  _total_volume = 0.0f;

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
          PhysShape *shape, PN_stdfloat mass_bias, PN_stdfloat rot_damping, PN_stdfloat density,
          PN_stdfloat damping, PN_stdfloat thickness, PN_stdfloat inertia,
          const LVecBase2 &limit_x, const LVecBase2 &limit_y, const LVecBase2 &limit_z) {

  physx::PxShape *pxshape = shape->get_shape();
  nassertv(pxshape->getGeometryType() == physx::PxGeometryType::eCONVEXMESH);
  physx::PxConvexMeshGeometry geom;
  pxshape->getConvexMeshGeometry(geom);
  physx::PxReal volume;
  physx::PxMat33 it;
  physx::PxVec3 com;
  geom.convexMesh->getMassInformation(volume, it, com);

  PT(Joint) joint = new Joint;
  if (!parent.empty()) {
    joint->parent = _joints[parent];
  } else {
    joint->parent = nullptr;
  }
  joint->joint = _char->find_joint(child);

  joint->mass = 1.0f;
  joint->damping = damping;
  joint->angular_damping = rot_damping;
  joint->inertia = inertia;
  joint->mass_bias = mass_bias;
  joint->density = density;
  joint->volume = volume;
  //joint->surface_area = get_surface_area(geom);

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
compute_mass() {
  if (_total_mass == 0.0) {
    for (size_t i = 0; i < _all_joints.size(); i++) {
      Joint *joint = _all_joints[i];
      //if (joint->thickness > 0) {
      //  _total_mass += joint->surface_area * joint->thickness * CUBIC_METERS_PER_CUBIC_INCH * joint->density;
      //} else {
        _total_mass += joint->volume * joint->density;
      //}
    }
  }

  _total_volume = 0.0;
  for (size_t i = 0; i < _all_joints.size(); i++) {
    Joint *joint = _all_joints[i];
    _total_volume += joint->volume * joint->mass_bias;
  }

  for (size_t i = 0; i < _all_joints.size(); i++) {
    Joint *joint = _all_joints[i];
    joint->mass = ((joint->volume * joint->mass_bias) / _total_volume) * _total_mass;
    if (joint->mass < 1.0) {
      joint->mass = 1.0;
    }
  }
}

/**
 *
 */
void PhysRagdoll::
create_joints() {
  compute_mass();

  for (size_t i = 0; i < _all_joints.size(); i++) {
    Joint *joint = _all_joints[i];

    CPT(TransformState) joint_pose = joint_default_net_transform(joint->joint);

    joint->actor = new PhysRigidDynamicNode(_char->get_joint_name(joint->joint));
    joint->actor->add_shape(joint->shape);
    //joint->actor->compute_mass_properties();
    joint->actor->set_mass(joint->mass);
    joint->actor->set_angular_damping(joint->angular_damping);
    joint->actor->set_transform(joint_pose);
    joint->actor->set_contact_callback(_contact_callback);
    physx::PxRigidDynamic *dyn = (physx::PxRigidDynamic *)joint->actor->get_rigid_body();
    dyn->setSolverIterationCounts(20, 20);

    if (joint->parent != nullptr) {
      CPT(TransformState) parent_pose = joint_default_net_transform(joint->parent->joint)->invert_compose(joint_pose);

      PT(PhysD6Joint) djoint = new PhysD6Joint(joint->parent->actor, joint->actor,
                                              parent_pose, TransformState::make_identity());
      djoint->set_linear_motion(PhysD6Joint::A_x, PhysD6Joint::M_locked);
      djoint->set_linear_motion(PhysD6Joint::A_y, PhysD6Joint::M_locked);
      djoint->set_linear_motion(PhysD6Joint::A_z, PhysD6Joint::M_locked);
      djoint->set_projection_enabled(true);
      ((physx::PxD6Joint *)djoint->get_joint())->setProjectionLinearTolerance(0.1f);
      ((physx::PxD6Joint *)djoint->get_joint())->setProjectionAngularTolerance(0.4f);
      djoint->set_collision_enabled(false);

      if (joint->limit_x[0] == 0 && joint->limit_x[1] == 0) {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_locked);

      } else if (joint->limit_x[0] == -180 && joint->limit_x[1] == 180) {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_free);

      } else {
        djoint->set_angular_motion(PhysD6Joint::A_x, PhysD6Joint::M_limited);
        djoint->set_twist_limit(PhysJointLimitAngularPair(joint->limit_x[0], joint->limit_x[1], joint->limit_x[1]));
      }

      djoint->set_angular_motion(PhysD6Joint::A_y, PhysD6Joint::M_limited);
      djoint->set_angular_motion(PhysD6Joint::A_z, PhysD6Joint::M_limited);
      djoint->set_pyramid_swing_limit(
        PhysJointLimitPyramid(joint->limit_y[0], joint->limit_y[1], joint->limit_z[0], joint->limit_z[1], std::max(joint->limit_y[1], joint->limit_z[1])));

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
  for (size_t i = 0; i < _all_joints.size(); i++) {
    _aggregate->removeActor(*(_all_joints[i]->actor->get_rigid_actor()));
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
  if (_aggregate->getScene() != nullptr) {
    _aggregate->getScene()->removeAggregate(*_aggregate);
  }

  clear_joints();

  _aggregate->release();
  _aggregate = nullptr;

  _enabled = false;
}

/**
 *
 */
void PhysRagdoll::
set_total_mass(PN_stdfloat mass) {
  _total_mass = mass;
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

    if (limb == nullptr) {
      _char->set_joint_forced_value(i, _char->get_joint_default_value(i));
      continue;
    }

    LMatrix4 net_inverse = LMatrix4::ident_mat();
    int parent = _char->get_joint_parent(limb->joint);
    if (parent != -1) {
      net_inverse = _char->get_joint_net_transform(parent);
    }
    net_inverse.invert_in_place();
    LMatrix4 joint_trans = char_net->invert_compose(
      limb->actor->get_transform())->get_mat();
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

  const PhysContactPair *pair = data->get_contact_pair(0);
  if (!pair->is_contact_type(PhysEnums::CT_found)) {
    return;
  }
  PhysContactPoint point = pair->get_contact_point(0);

  LVector3 force = point.get_impulse() / 0.015;
  PN_stdfloat force_magnitude = force.length();

  LPoint3 position = point.get_position();

  //std::cout << "Ragdoll contact between " << NodePath(data->get_actor_a()) << " and " << NodePath(data->get_actor_b()) << " force " << force_magnitude << "\n";

  Randomizer random;

  if (force_magnitude >= _ragdoll->_hard_impact_force) {
    int index = random.random_int(_ragdoll->_hard_impact_sounds.size());
    _ragdoll->_hard_impact_sounds[index]->set_3d_attributes(
      position[0], position[1], position[2], 0.0, 0.0, 0.0);
    _ragdoll->_hard_impact_sounds[index]->play();
  } else if (force_magnitude >= _ragdoll->_soft_impact_force) {
    int index = random.random_int(_ragdoll->_soft_impact_sounds.size());
    _ragdoll->_soft_impact_sounds[index]->set_3d_attributes(
      position[0], position[1], position[2], 0.0, 0.0, 0.0);
    _ragdoll->_soft_impact_sounds[index]->play();
  }
}
