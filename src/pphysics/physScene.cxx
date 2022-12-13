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
#include "physGeometry.h"
#include "clockObject.h"
#include "jobSystem.h"

/**
 *
 */
PhysScene::
PhysScene() :
  _local_time(0.0),
  _last_frame_time(0.0),
  _tick_count(0),
  _max_substeps(10),
  _fixed_timestep(1 / 60.0),
  _debug_vis_enabled(false)
{
  PhysSystem *sys = PhysSystem::ptr();
  physx::PxSceneDesc desc(sys->get_scale());
  desc.cpuDispatcher = sys->get_cpu_dispatcher();
  // Enable this flag so we know which actors changed each time we simulate, so
  // we can update the transform of the associated nodes.
  desc.flags = desc.flags | physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS | physx::PxSceneFlag::eENABLE_CCD;
  desc.ccdMaxPasses = 5;
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
simulate(double frame_time) {

  _global_contact_queue.clear();

  JobSystem *jsys = JobSystem::get_global_ptr();

  ClockObject *clock = ClockObject::get_global_clock();
  ClockObject::Mode save_mode = clock->get_mode();
  clock->set_mode(ClockObject::M_slave);

  double dt = frame_time - _last_frame_time;
  _last_frame_time = frame_time;
  _local_time += dt;

  int num_steps = 0;
  if (_local_time >= _fixed_timestep) {
    num_steps = (int)(_local_time / _fixed_timestep);
    _local_time -= num_steps * _fixed_timestep;
  }

  if (num_steps > 0) {
    num_steps = std::min(num_steps, _max_substeps);

    for (int i = 0; i < num_steps; ++i) {
      //_global_contact_queue.clear();

      double sim_time = _tick_count * _fixed_timestep;
      clock->set_frame_time(sim_time);

      _scene->simulate(_fixed_timestep);
      _scene->fetchResults(true);

      // Record transforms of active actors in interpolation history.
      physx::PxU32 num_active_actors;
      physx::PxActor **active_actors = _scene->getActiveActors(num_active_actors);

      //jsys->parallel_process(num_active_actors,
      //  [&] (int i) {
      for (int i = 0; i < num_active_actors; ++i) {
          physx::PxActor *actor = active_actors[i];

          if (!actor->is<physx::PxRigidActor>()) {
            continue;
          }

          physx::PxRigidActor *rigid_actor = (physx::PxRigidActor *)actor;

          if (actor->is<physx::PxRigidBody>()) {
            physx::PxRigidBody *rigid_body = (physx::PxRigidBody *)rigid_actor;
            if (rigid_body->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
              continue;
            }
          }

          PhysRigidActorNode *node = (PhysRigidActorNode *)actor->userData;
          if (node == nullptr) {
            continue;
          }

          // Disable automatic syncing with PhysX when the node's transform changes.
          // We are doing the exact opposite here, synchronizing PhysX's transform
          // with the node.
          //node->set_sync_enabled(false);

          NodePath np(node);

          physx::PxTransform global_pose = rigid_actor->getGlobalPose();

          // Update the local-space transform of the node.
          if (np.get_parent().is_empty()) {
            // Has no parent!  Just throw the global pose on there.
            bool pos_changed = node->_iv_pos.record_value(physx_vec_to_panda(global_pose.p), sim_time, false);
            bool rot_changed = node->_iv_rot.record_value(physx_quat_to_panda(global_pose.q), sim_time, false);
            if (pos_changed || rot_changed) {
              node->_needs_interpolation = true;
            }

          } else {
            // The global pose needs to be transformed into the local coordinate
            // space of the associate node's parent.
            NodePath parent = np.get_parent();

            CPT(TransformState) global_ts = physx_trans_to_panda(global_pose);

            CPT(TransformState) parent_net = parent.get_net_transform();

            CPT(TransformState) local_ts = parent_net->invert_compose(global_ts);
            bool pos_changed = node->_iv_pos.record_value(local_ts->get_pos(), sim_time, false);
            bool rot_changed = node->_iv_rot.record_value(local_ts->get_norm_quat(), sim_time, false);
            if (pos_changed || rot_changed) {
              node->_needs_interpolation = true;
            }
          }
        }
      //);

      _tick_count++;

      // TODO: Should we wait until after the simulation to run the callbacks,
      // so time is restored correctly?
      run_callbacks();
    }

    clock->set_frame_time(frame_time);
  }

  clock->set_mode(save_mode);

  if (_tick_count > 0) {
    // Interpolate actor transforms for the true rendering time.
    double interp_time = (_tick_count - 1) * _fixed_timestep;
    interp_time -= _fixed_timestep;
    interp_time += _local_time;
    interp_time = std::max(0.0, interp_time);

    //jsys->parallel_process(_actors.size(),
      //[&] (int i) {
      for (int i = 0; i < _actors.size(); ++i) {
        PhysRigidActorNode *actor = _actors[i];

        if (!actor->_needs_interpolation) {
          continue;
        }

        int pret = actor->_iv_pos.interpolate(interp_time);
        int rret = actor->_iv_rot.interpolate(interp_time);
        if (pret && rret) {
          actor->_needs_interpolation = false;
        }

        CPT(TransformState) ts = TransformState::make_pos_quat(
          actor->_iv_pos.get_interpolated_value(),
          actor->_iv_rot.get_interpolated_value());
        actor->set_sync_enabled(false);
        actor->set_transform(ts);
        actor->set_sync_enabled(true);
      }
    //);
  }

  return num_steps;
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
        CallbackObject *filter) const {

  physx::PxQueryFilterData data;
  data.flags |= physx::PxQueryFlag::ePREFILTER;
  // word0 is used during the fixed-function filtering.
  data.data.word0 = (block_mask | touch_mask).get_word();
  data.data.word1 = block_mask.get_word();
  data.data.word2 = touch_mask.get_word();

  PhysBaseQueryFilter pfilter(filter);
  return _scene->raycast(
    panda_vec_to_physx(origin),
    panda_norm_vec_to_physx(direction),
    panda_length_to_physx(distance),
    result.get_buffer(),
    physx::PxHitFlags(physx::PxHitFlag::eDEFAULT | physx::PxHitFlag::eMTD | physx::PxHitFlag::eMESH_BOTH_SIDES),
    data,
    &pfilter);
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
        CallbackObject *filter) const {

  physx::PxQueryFilterData data;
  data.flags |= physx::PxQueryFlag::ePREFILTER;
  // word0 is used during the fixed-function filtering.
  data.data.word0 = (solid_mask | touch_mask).get_word();
  data.data.word1 = solid_mask.get_word();
  data.data.word2 = touch_mask.get_word();

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

  PhysBaseQueryFilter pfilter(filter);
  return _scene->sweep(
    box, trans,
    panda_norm_vec_to_physx(direction),
    panda_length_to_physx(distance),
    result.get_buffer(),
    physx::PxHitFlags(physx::PxHitFlag::eDEFAULT | physx::PxHitFlag::eMTD | physx::PxHitFlag::eMESH_BOTH_SIDES),
    data,
    &pfilter);
}

/**
 * Casts a generic physics geometry object into the scene and records the
 * intersections.
 *
 * block_mask is the bitmask of collision groups that should prevent the ray
 * from continuing, while touch_mask is the bitmask of collision groups that
 * should allow the ray to continue (but still record an intersection).
 *
 * Returns true if there was at least one intersection, false otherwise.
 */
bool PhysScene::
sweep(PhysSweepResult &result, PhysGeometry &geometry,
      const LPoint3 &pos, const LVecBase3 &hpr,
      const LVector3 &direction, PN_stdfloat distance,
      CollideMask solid_mask, CollideMask touch_mask,
      CallbackObject *filter) const {

  physx::PxQueryFilterData data;
  data.flags |= physx::PxQueryFlag::ePREFILTER;
  // word0 is used during the fixed-function filtering.
  data.data.word0 = (solid_mask | touch_mask).get_word();
  data.data.word1 = solid_mask.get_word();
  data.data.word2 = touch_mask.get_word();

  physx::PxTransform trans;
  trans.p.x = panda_length_to_physx(pos[0]);
  trans.p.y = panda_length_to_physx(pos[1]);
  trans.p.z = panda_length_to_physx(pos[2]);
  LQuaternion quat;
  quat.set_hpr(hpr);
  trans.q = panda_quat_to_physx(quat);

  PhysBaseQueryFilter pfilter(filter);
  return _scene->sweep(
    *geometry.get_geometry(), trans,
    panda_norm_vec_to_physx(direction),
    panda_length_to_physx(distance),
    result.get_buffer(),
    physx::PxHitFlags(physx::PxHitFlag::eDEFAULT | physx::PxHitFlag::eMTD | physx::PxHitFlag::eMESH_BOTH_SIDES),
    data,
    &pfilter);
}

/**
 *
 */
void PhysScene::
run_callbacks() {
  for (CallbackQueue::const_iterator it = _callbacks.begin();
       it != _callbacks.end(); ++it) {
    const Callback &clbk = *it;
    if (!clbk._data->is_valid()) {
      // A previous callback may have deleted the nodes pertaining
      // to this callback.  Guard against that.
#ifndef NDEBUG
      if (pphysics_cat.is_debug()) {
        pphysics_cat.debug()
          << "Aborting callback with deleted nodes\n";
      }
#endif
      continue;
    }
    clbk._callback->do_callback(clbk._data);
  }

  _callbacks.clear();
}
