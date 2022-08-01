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

ParticleManager2 *ParticleManager2::_global_ptr = nullptr;

/**
 *
 */
void ParticleManager2::
update(double dt) {
  for (Systems::const_iterator it = _systems.begin(); it != _systems.end();) {
    ParticleSystem2 *system = *it;
    if (!system->update(dt)) {
      // Soft-stop completed during update, remove system.
      it = _systems.erase(it);
    } else {
      ++it;
    }
  }
}

/**
 *
 */
void ParticleManager2::
add_system(ParticleSystem2 *system) {
  _systems.insert(system);
}

/**
 *
 */
void ParticleManager2::
remove_system(ParticleSystem2 *system) {
  nassertv(!system->is_running());
  _systems.erase(system);
}

/**
 *
 */
void ParticleManager2::
stop_and_remove_all_systems() {
  for (ParticleSystem2 *system : _systems) {
    system->stop();
  }
  _systems.clear();
}
