/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particlePool2.I
 * @author brian
 * @date 2022-08-04
 */

/**
 * Loads the particle system(s) from the indicated filename and stores them
 * for future by-name lookup.
 *
 * Returns true if a particle system was loaded successfully, or false
 * otherwise.
 */
INLINE bool ParticlePool2::
load_particles(const Filename &filename, const DSearchPath &search_path) {
  return get_global_ptr()->ns_load_particles(filename, search_path);
}

/**
 * Returns a copy of a previously loaded particle system with the indicated
 * name, or nullptr if there is no particle system in the pool with such
 * name.  The caller has ownership of the copy.
 */
INLINE PT(ParticleSystem2) ParticlePool2::
get_particle_system(const std::string &name) {
  return get_global_ptr()->ns_get_particle_system(name);
}

/**
 * Removes the particle system with the indicated name from the pool.
 */
INLINE void ParticlePool2::
remove_particle_system(const std::string &name) {
  get_global_ptr()->ns_remove_particle_system(name);
}

/**
 * Removes all previously loaded particle systems from the pool.
 */
INLINE void ParticlePool2::
remove_all_particle_systems() {
  get_global_ptr()->ns_remove_all_particle_systems();
}

/**
 *
 */
INLINE ParticlePool2 *ParticlePool2::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new ParticlePool2;
  }
  return _global_ptr;
}
