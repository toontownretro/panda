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
#include "physRayCastResult.h"
#include "physSweepResult.h"
#include "physXSimulationEventCallback.h"
#include "physQueryFilter.h"
#include "physx_utils.h"

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
  desc.filterShader = PandaSimulationFilterShader::filter;
  desc.filterCallback = PandaSimulationFilterCallback::ptr();
  desc.simulationEventCallback = new PhysXSimulationEventCallback(this);
  switch (phys_solver.get_value()) {
  default:
  case PST_pgs:
    desc.solverType = physx::PxSolverType::ePGS;
    break;
  case PST_tgs:
    desc.solverType = physx::PxSolverType::eTGS;
    break;
  }
  _scene = sys->get_physics()->createScene(desc);
  _scene->userData = this;

  physx::PxPvdSceneClient *pvd_client = _scene->getScenePvdClient();
  if (pvd_client != nullptr) {
    pvd_client->setScenePvdFlags(
      physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS |
      physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS |
      physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES);
  }

  _controller_mgr = PxCreateControllerManager(*_scene);
}

/**
 *
 */
PhysScene::
~PhysScene() {
  if (_controller_mgr != nullptr) {
    _controller_mgr->release();
    _controller_mgr = nullptr;
  }

  if (_scene != nullptr) {
    _scene->userData = nullptr;

    delete _scene->getSimulationEventCallback();
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

  // Don't simulate less than 0.1 ms or more than 1 sec.
  if (dt < 1.0 && dt > 0.0001) {
    if (dt > 0.1) {
      // Cap at 100 ms.
      dt = 0.1;
    }

    _local_time += dt;
    _scene->simulate(dt);
    _scene->fetchResults(true);

    // Now synchronize the simulation results with all of the scene nodes.
    physx::PxU32 num_active_actors;
    physx::PxActor **active_actors = _scene->getActiveActors(num_active_actors);
    //if (pphysics_cat.is_debug()) {
    //  pphysics_cat.debug()
    //    << num_active_actors << " active actors this sim\n";
    //}
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

      //if (pphysics_cat.is_debug()) {
      //  pphysics_cat.debug()
      //    << "Global pose for rigid actor connected to " << np << ": "
      //    << "pos: " << global_pose.p.x << ", " << global_pose.p.y << ", " << global_pose.p.z
      //   << " | quat: " << global_pose.q.x << ", " << global_pose.q.y << ", " << global_pose.q.z << ", " << global_pose.q.w << "\n";
      //}

      // Update the local-space transform of the node.
      if (np.get_parent().is_empty()) {
        // Has no parent!  Just throw the global pose on there.
        CPT(TransformState) ts = node->get_transform();
        ts = ts->set_pos(physx_vec_to_panda(global_pose.p));
        ts = ts->set_quat(physx_quat_to_panda(global_pose.q));
        node->set_transform(ts);

      } else {
        // The global pose needs to be transformed into the local coordinate
        // space of the associate node's parent.
        NodePath parent = np.get_parent();

        CPT(TransformState) curr_ts = node->get_transform();

        CPT(TransformState) global_ts = physx_trans_to_panda(global_pose);

        CPT(TransformState) parent_net = parent.get_net_transform();

        CPT(TransformState) local_ts = parent_net->invert_compose(global_ts);
        local_ts = local_ts->set_scale(curr_ts->get_scale());
        local_ts = local_ts->set_shear(curr_ts->get_shear());

        node->set_transform(local_ts);
      }

      node->set_sync_enabled(true);
    }

    run_callbacks();

    return 1;
  }

  return 0;
}

/**
 * Casts a ray into the scene and records the intersections that were found.
 *
 * block_mask is the bitmask of collision groups that should prevent the ray
 * from continuing, while touch_mask is the bitmask of collision groups that
 * should allow the ray to continue (but still record an intersection).
 */
bool PhysScene::
raycast(PhysRayCastResult &result, const LPoint3 &origin,
        const LVector3 &direction, PN_stdfloat distance,
        CollideMask block_mask, CollideMask touch_mask,
        unsigned int collision_group,
        PhysBaseQueryFilter *filter) const {

  physx::PxQueryFilterData data;
  data.flags |= physx::PxQueryFlag::ePREFILTER;
  // word0 is used during the fixed-function filtering.
  data.data.word0 = (block_mask | touch_mask).get_word();
  data.data.word1 = block_mask.get_word();
  data.data.word2 = touch_mask.get_word();
  data.data.word3 = collision_group;

  if (filter == nullptr) {
    // Use the base filter that just checks for common block or touch bits.
    PhysBaseQueryFilter default_filter;
    return _scene->raycast(
      panda_vec_to_physx(origin),
      panda_norm_vec_to_physx(direction),
      panda_length_to_physx(distance),
      result.get_buffer(),
      physx::PxHitFlags(physx::PxHitFlag::eDEFAULT),
      data,
      &default_filter);

  } else {
    // Explicit filter was specified.
    return _scene->raycast(
      panda_vec_to_physx(origin),
      panda_norm_vec_to_physx(direction),
      panda_length_to_physx(distance),
      result.get_buffer(),
      physx::PxHitFlags(physx::PxHitFlag::eDEFAULT),
      data,
      filter);
  }
}

/**
 * Casts a bounding box into the scene and records the intersections.
 *
 * block_mask is the bitmask of collision groups that should prevent the ray
 * from continuing, while touch_mask is the bitmask of collision groups that
 * should allow the ray to continue (but still record an intersection).
 *
 * Returns true if there was at least one intersection, false otherwise.
 */
bool PhysScene::
boxcast(PhysSweepResult &result, const LPoint3 &mins, const LPoint3 &maxs,
        const LVector3 &direction, PN_stdfloat distance,
        const LVecBase3 &hpr,
        CollideMask solid_mask, CollideMask touch_mask,
        unsigned int collision_group, PhysBaseQueryFilter *filter) const {

  physx::PxQueryFilterData data;
  data.flags |= physx::PxQueryFlag::ePREFILTER;
  // word0 is used during the fixed-function filtering.
  data.data.word0 = (solid_mask | touch_mask).get_word();
  data.data.word1 = solid_mask.get_word();
  data.data.word2 = touch_mask.get_word();
  data.data.word3 = collision_group;

  PN_stdfloat hx, hy, hz, cx, cy, cz;
  hx = panda_length_to_physx((maxs[0] - mins[0]) / 2.0f);
  hy = panda_length_to_physx((maxs[1] - mins[1]) / 2.0f);
  hz = panda_length_to_physx((maxs[2] - mins[2]) / 2.0f);
  cx = panda_length_to_physx((maxs[0] + mins[0]) / 2.0f);
  cy = panda_length_to_physx((maxs[1] + mins[1]) / 2.0f);
  cz = panda_length_to_physx((maxs[2] + mins[2]) / 2.0f);

  physx::PxBoxGeometry box;
  box.halfExtents.x = hx;
  box.halfExtents.y = hy;
  box.halfExtents.z = hz;
  physx::PxTransform trans;
  trans.p.x = cx;
  trans.p.y = cy;
  trans.p.z = cz;
  LQuaternion quat;
  quat.set_hpr(hpr);
  trans.q = panda_quat_to_physx(quat);

  if (filter == nullptr) {
    // Use the base filter that just checks for common block or touch bits.
    PhysBaseQueryFilter default_filter;
    return _scene->sweep(
      box, trans,
      panda_norm_vec_to_physx(direction),
      panda_length_to_physx(distance),
      result.get_buffer(),
      physx::PxHitFlags(physx::PxHitFlag::eDEFAULT),
      data,
      &default_filter);

  } else {
    // Explicit filter was specified.
    return _scene->sweep(
      box, trans,
      panda_norm_vec_to_physx(direction),
      panda_length_to_physx(distance),
      result.get_buffer(),
      physx::PxHitFlags(physx::PxHitFlag::eDEFAULT),
      data,
      filter);
  }
}

/**
 *
 */
void PhysScene::
run_callbacks() {
  for (CallbackQueue::const_iterator it = _callbacks.begin();
       it != _callbacks.end(); ++it) {
    const Callback &clbk = *it;
    clbk._callback->do_callback(clbk._data);
  }

  _callbacks.clear();
}
