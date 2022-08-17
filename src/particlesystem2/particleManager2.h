/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleManager2.h
 * @author brian
 * @date 2022-07-31
 */

#ifndef PARTICLEMANAGER2_H
#define PARTICLEMANAGER2_H

#include "pandabase.h"
#include "pvector.h"
#include "particleSystem2.h"
#include "pointerTo.h"
#include "lightMutex.h"

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticleManager2 {
PUBLISHED:
  INLINE static ParticleManager2 *get_global_ptr();

  void update(double dt);

  void stop_and_remove_all_systems();

public:
  INLINE ParticleManager2();

  void add_system(ParticleSystem2 *system);
  void remove_system(ParticleSystem2 *system);

private:
  typedef pvector<PT(ParticleSystem2)> Systems;
  // All active systems.
  Systems _systems;

  LightMutex _removed_systems_lock;

private:
  static ParticleManager2 *_global_ptr;
};

#include "particleManager2.I"

#endif // PARTICLEMANAGER2_H
