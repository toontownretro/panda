/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particlePool2.cxx
 * @author brian
 * @date 2022-08-04
 */

#include "particlePool2.h"
#include "virtualFileSystem.h"
#include "config_particlesystem2.h"
#include "datagramInputFile.h"
#include "bam.h"
#include "bamReader.h"
#include "typedWritable.h"

ParticlePool2 *ParticlePool2::_global_ptr = nullptr;

/**
 * Loads the particle system(s) from the indicated filename and stores them
 * for future by-name lookup.
 *
 * Returns true if a particle system was loaded successfully, or false
 * otherwise.
 */
bool ParticlePool2::
ns_load_particles(const Filename &filename, const DSearchPath &search_path) {
  if (filename.get_extension() != "pto") {
    particlesystem2_cat.error()
      << "Particle system filename must have extension .pto!  Text format "
      << "particle system definitions not yet supported.\n";
    return false;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename resolved = filename;
  if (!vfs->resolve_filename(resolved, search_path)) {
    particlesystem2_cat.error()
      << "Failed to resolve particle system filename " << filename << " on search path "
      << search_path << "\n";
    return false;
  }

  DatagramInputFile din;
  if (!din.open(resolved)) {
    return false;
  }

  std::string head;
  if (!din.read_header(head, _bam_header.size())) {
    return false;
  }

  if (head != _bam_header) {
    return false;
  }

  BamReader reader(&din);
  if (!reader.init()) {
    return false;
  }

  TypedWritable *obj = reader.read_object();

  if (obj == nullptr || !reader.resolve()) {
    return false;
  }

  if (!obj->is_of_type(ParticleSystem2::get_class_type())) {
    particlesystem2_cat.error()
      << "File " << resolved << " does not contain a particle system!\n";
    return false;
  }

  PT(ParticleSystem2) sys = DCAST(ParticleSystem2, obj);
  _systems_by_name[sys->get_name()] = sys;

  return true;
}

/**
 * Returns a copy of a previously loaded particle system with the indicated
 * name, or nullptr if there is no particle system in the pool with such
 * name.  The caller has ownership of the copy.
 */
PT(ParticleSystem2) ParticlePool2::
ns_get_particle_system(const std::string &name) const {
  SystemsByName::const_iterator it = _systems_by_name.find(name);
  if (it != _systems_by_name.end()) {
    return (*it).second->make_copy();
  }
  return nullptr;
}

/**
 * Removes the particle system with the indicated name from the pool.
 */
void ParticlePool2::
ns_remove_particle_system(const std::string &name) {
  SystemsByName::const_iterator it = _systems_by_name.find(name);
  if (it != _systems_by_name.end()) {
    _systems_by_name.erase(it);
  }
}

/**
 * Removes all previously loaded particle systems from the pool.
 */
void ParticlePool2::
ns_remove_all_particle_systems() {
  _systems_by_name.clear();
}
