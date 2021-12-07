/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physScene.h
 * @author brian
 * @date 2021-04-14
 */

#ifndef PHYSSCENE_H
#define PHYSSCENE_H

#include "pandabase.h"
#include "referenceCount.h"
#include "luse.h"
#include "collideMask.h"
#include "pdeque.h"
#include "physx_shaders.h"
#include "refCallbackData.h"
#include "callbackObject.h"

#include "physx_utils.h"

class PhysRayCastResult;
class PhysSweepResult;
class PhysBaseQueryFilter;
class PhysGeometry;

/**
 * A scene is a collection of bodies and constraints which can interact.
 * The scene simulates the behavior of these objects over time.  Several scenes
 * may exis at the same time, but each body or constraint is specific to a
 * scene -- they may not be shared.
 */
class EXPCL_PANDA_PPHYSICS PhysScene : public ReferenceCount {
PUBLISHED:
  PhysScene();
  ~PhysScene();

  int simulate(double dt);

  INLINE void set_gravity(const LVector3 &gravity);
  INLINE LVector3 get_gravity() const;

  INLINE void shift_origin(const LVector3 &shift);

  INLINE void set_fixed_timestep(double step);
  INLINE double get_fixed_timestep() const;

  INLINE void set_max_substeps(int count);
  INLINE int get_max_substeps() const;

  INLINE void set_group_collision_flag(int a, int b, bool enable);
  INLINE bool get_group_collision_flag(int a, int b) const;

  bool raycast(PhysRayCastResult &result, const LPoint3 &origin,
               const LVector3 &direction, PN_stdfloat distance,
               CollideMask solid_mask = CollideMask::all_on(),
               CollideMask touch_mask = CollideMask::all_off(),
               unsigned int collision_group = 0,
               PhysBaseQueryFilter *filter = nullptr) const;
  bool boxcast(PhysSweepResult &result, const LPoint3 &mins, const LPoint3 &maxs,
               const LVector3 &direction, PN_stdfloat distance,
               const LVecBase3 &hpr = LVecBase3(0),
               CollideMask solid_mask = CollideMask::all_on(),
               CollideMask touch_mask = CollideMask::all_off(),
               unsigned int collision_group = 0,
               PhysBaseQueryFilter *filter = nullptr) const;
  bool sweep(PhysSweepResult &result, PhysGeometry &geometry,
             const LPoint3 &pos, const LVecBase3 &hpr,
             const LVector3 &direction, PN_stdfloat distance,
             CollideMask solid_mask = CollideMask::all_on(),
             CollideMask touch_mask = CollideMask::all_off(),
             unsigned int collision_group = 0,
             PhysBaseQueryFilter *filter = nullptr) const;

public:
  INLINE void enqueue_callback(CallbackObject *obj, RefCallbackData *data);

  INLINE physx::PxScene *get_scene() const;
  INLINE physx::PxControllerManager *get_controller_manager() const;

private:
  void run_callbacks();

private:
  class Callback {
  public:
    PT(RefCallbackData) _data;
    PT(CallbackObject) _callback;
  };
  typedef pdeque<Callback> CallbackQueue;
  CallbackQueue _callbacks;

  double _local_time;
  int _max_substeps;
  double _fixed_timestep;

  physx::PxScene *_scene;
  physx::PxControllerManager *_controller_mgr;

  bool _debug_vis_enabled;
};

#include "physScene.I"

#endif // PHYSSCENE_H
