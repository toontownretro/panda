/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleManager2.cxx
 * @author brian
 * @date 2022-07-31
 */

#include "particleManager2.h"
#include "jobSystem.h"

ParticleManager2 *ParticleManager2::_global_ptr = nullptr;

/**
 *
 */
void ParticleManager2::
update(double dt) {
  if (_systems.empty()) {
    return;
  }

  JobSystem *jsys = JobSystem::get_global_ptr();

  pvector<ParticleSystem2 *> removed_systems;
  jsys->parallel_process(_systems.size(),
    [&removed_systems, dt, this](int i) {
      ParticleSystem2 *system = _systems[i];
      if (!system->update(dt)) {
        _removed_systems_lock.acquire();
        removed_systems.push_back(system);
        _removed_systems_lock.release();
      }
    }
  );

  // Remove systems that stopped during parallel update.
  for (size_t i = 0; i < removed_systems.size(); ++i) {
    ParticleSystem2 *system = removed_systems[i];
    Systems::const_iterator it = std::find(_systems.begin(), _systems.end(), system);
    assert(it != _systems.end());
    _systems.erase(it);
  }
}

/**
 *
 */
void ParticleManager2::
add_system(ParticleSystem2 *system) {
  _systems.push_back(system);
}

/**
 *
 */
void ParticleManager2::
remove_system(ParticleSystem2 *system) {
  nassertv(!system->is_running());
  Systems::const_iterator it = std::find(_systems.begin(), _systems.end(), system);
  assert(it != _systems.end());
  _systems.erase(it);
}

/**
 *
 */
void ParticleManager2::
stop_and_remove_all_systems() {
  for (ParticleSystem2 *system : _systems) {
    system->priv_stop();
  }
  _systems.clear();
}
