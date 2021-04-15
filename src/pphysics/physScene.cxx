/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physScene.cxx
 * @author brian
 * @date 2021-04-14
 */

#include "config_pphysics.h"
#include "physScene.h"
#include "physSystem.h"
#include "physRigidActorNode.h"
#include "nodePath.h"

/**
 *
 */
PhysScene::
PhysScene() :
  _local_time(0.0),
  _max_substeps(10),
  _fixed_timestep(1 / 60.0),
  _debug_vis_enabled(false)
{
  PhysSystem *sys = PhysSystem::ptr();
  physx::PxSceneDesc desc(sys->get_scale());
  desc.cpuDispatcher = sys->get_cpu_dispatcher();
  // Enable this flag so we know which actors changed each time we simulate, so
  // we can update the transform of the associated nodes.
  desc.flags = desc.flags | physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
  desc.filterShader = physx::PxDefaultSimulationFilterShader;
  _scene = sys->get_physics()->createScene(desc);
  _scene->userData = this;
}

/**
 *
 */
PhysScene::
~PhysScene() {
  if (_scene != nullptr) {
    _scene->release();
    _scene = nullptr;
  }
}

/**
 * Requests a physics simulation step.  dt is the time elapsed in seconds since
 * the previous frame.  May run 0, 1, or N simulation steps, depending on the
 * configured maximum number of substeps and the fixed timestep.
 *
 * Returns the number of simulation steps that were run.
 */
int PhysScene::
simulate(double dt) {
  int num_substeps = 0;

  if (_max_substeps > 0) {
    // Fixed timestep with interpolation.
    _local_time += dt;
    if (_local_time >= _fixed_timestep) {
      num_substeps = (int)(_local_time / _fixed_timestep);
      _local_time -= num_substeps * _fixed_timestep;
    }

  } else {
    // Variable timestep.
    _local_time = dt;
    _fixed_timestep = dt;
    if (IS_NEARLY_ZERO(dt)) {
      num_substeps = 0;
      _max_substeps = 0;

    } else {
      num_substeps = 1;
      _max_substeps = 1;
    }
  }

  if (num_substeps > 0) {
    // Clamp the number of substeps, to prevent simulation from grinding down
    // to a halt if we can't keep up.
    int clamped_substeps = (num_substeps > _max_substeps) ? _max_substeps : num_substeps;

    for (int i = 0; i < clamped_substeps; i++) {
      _scene->simulate(_fixed_timestep);
      _scene->fetchResults(true);
    }
  }

  // Now synchronize the simulation results with all of the scene nodes.
  physx::PxU32 num_active_actors;
  physx::PxActor **active_actors = _scene->getActiveActors(num_active_actors);
  if (pphysics_cat.is_debug()) {
    pphysics_cat.debug()
      << num_active_actors << " active actors this sim\n";
  }
  for (physx::PxU32 i = 0; i < num_active_actors; i++) {
    physx::PxActor *actor = active_actors[i];

    if (!actor->is<physx::PxRigidActor>()) {
      continue;
    }

    physx::PxRigidActor *rigid_actor = (physx::PxRigidActor *)actor;

    PhysRigidActorNode *node = (PhysRigidActorNode *)actor->userData;
    if (node == nullptr) {
      continue;
    }

    // Disable automatic syncing with PhysX when the node's transform changes.
    // We are doing the exact opposite here, synchronizing PhysX's transform
    // with the node.
    node->set_sync_enabled(false);

    NodePath np(node);

    physx::PxTransform global_pose = rigid_actor->getGlobalPose();

    if (pphysics_cat.is_debug()) {
      pphysics_cat.debug()
        << "Global pose for rigid actor connected to " << np << ": "
        << "pos: " << global_pose.p.x << ", " << global_pose.p.y << ", " << global_pose.p.z
        << " | quat: " << global_pose.q.x << ", " << global_pose.q.y << ", " << global_pose.q.z << ", " << global_pose.q.w << "\n";
    }

    // Update the local-space transform of the node.
    if (np.get_parent().is_empty()) {
      // Has no parent!  Just throw the global pose on there.
      CPT(TransformState) ts = node->get_transform();
      ts = ts->set_pos(LVecBase3(global_pose.p.x, global_pose.p.y, global_pose.p.z));
      ts = ts->set_quat(LQuaternion(global_pose.q.w, global_pose.q.x, global_pose.q.y, global_pose.q.z));
      node->set_transform(ts);

    } else {
      // The global pose needs to be transformed into the local coordinate
      // space of the associate node's parent.
      NodePath parent = np.get_parent();

      CPT(TransformState) curr_ts = node->get_transform();

      CPT(TransformState) global_ts = TransformState::make_pos_quat(
        LVecBase3(global_pose.p.x, global_pose.p.y, global_pose.p.z),
        LQuaternion(global_pose.q.w, global_pose.q.x, global_pose.q.y, global_pose.q.z));

      CPT(TransformState) parent_net = parent.get_net_transform();

      CPT(TransformState) local_ts = parent_net->invert_compose(global_ts);
      local_ts = local_ts->set_scale(curr_ts->get_scale());
      local_ts = local_ts->set_shear(curr_ts->get_shear());

      node->set_transform(local_ts);
    }

    node->set_sync_enabled(true);
  }

  return num_substeps;
}
