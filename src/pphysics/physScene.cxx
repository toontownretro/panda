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

#include "physScene.h"
#include "physSystem.h"

/**
 *
 */
PhysScene::
PhysScene() :
  _local_time(0.0),
  _max_substeps(10),
  _fixed_timestep(1 / 60.0)
{
  PhysSystem *sys = PhysSystem::ptr();
  physx::PxSceneDesc desc(sys->get_scale());
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
      num_substeps = (int)_local_time / _fixed_timestep;
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

  return num_substeps;
}
