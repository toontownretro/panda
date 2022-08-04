/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particlePool2.h
 * @author brian
 * @date 2022-08-04
 */

#ifndef PARTICLEPOOL2_H
#define PARTICLEPOOL2_H

#include "pandabase.h"
#include "particleSystem2.h"
#include "pointerTo.h"
#include "filename.h"
#include "dSearchPath.h"
#include "config_putil.h"
#include "pmap.h"

/**
 *
 */
class EXPCL_PANDA_PARTICLESYSTEM2 ParticlePool2 {
PUBLISHED:
  INLINE static bool load_particles(const Filename &filename, const DSearchPath &search_path = get_model_path());
  INLINE static PT(ParticleSystem2) get_particle_system(const std::string &name);
  INLINE static void remove_particle_system(const std::string &name);
  INLINE static void remove_all_particle_systems();

protected:
  ParticlePool2() = default;

  bool ns_load_particles(const Filename &filename, const DSearchPath &search_path);
  PT(ParticleSystem2) ns_get_particle_system(const std::string &name) const;
  void ns_remove_particle_system(const std::string &name);
  void ns_remove_all_particle_systems();

  INLINE static ParticlePool2 *get_global_ptr();

private:
  static ParticlePool2 *_global_ptr;

  typedef pmap<std::string, PT(ParticleSystem2)> SystemsByName;
  SystemsByName _systems_by_name;
};

#include "particlePool2.I"

#endif // PARTICLEPOOL2_H
