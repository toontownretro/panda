/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialPool.cxx
 * @author lachbr
 * @date 2021-03-17
 */

#include "materialPool.h"
#include "config_material.h"
#include "virtualFileSystem.h"
#include "keyValues.h"
#include "materialRegistry.h"
#include "datagramInputFile.h"
#include "bam.h"
#include "bamReader.h"

MaterialPool *MaterialPool::_global_ptr = nullptr;

/**
 *
 */
MaterialPool::
MaterialPool() {
}

/**
 *
 */
PT(MaterialBase) MaterialPool::
ns_load_material(const Filename &filename, const DSearchPath &search_path) {
  Materials::const_iterator it;

  it = _materials.find(filename);
  if (it != _materials.end()) {
    return (*it).second;
  }

  Filename fullpath = filename;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(fullpath, search_path)) {
    material_cat.error()
      << "Couldn't find material " << filename << " on search path "
      << search_path << "\n";
    return nullptr;
  }

  it = _fullpath_materials.find(fullpath);
  if (it != _fullpath_materials.end()) {
    return (*it).second;
  }

  // Not in cache, load it up.

  PT(MaterialBase) material;

  if (fullpath.get_extension() == "pmat") {
    // Keyvalues material file.

    PT(KeyValues) kv = KeyValues::load(fullpath);
    if (kv == nullptr) {
      return nullptr;
    }
    if (kv->get_num_children() != 1) {
      return nullptr;
    }

    KeyValues *mat_block = kv->get_child(0);
    const std::string &material_name = mat_block->get_name();

    material = MaterialRegistry::get_global_ptr()->create_material(material_name);
    if (material == nullptr) {
      return nullptr;
    }

    // Append the dirname of the material file to the search path.  This search
    // path will be used to locate textures referenced by the material.
    DSearchPath mat_search_path = search_path;
    mat_search_path.append_directory(fullpath.get_dirname());

    material->read_keyvalues(mat_block, mat_search_path);

  } else if (fullpath.get_extension() == "mto") {
    // Bam format material object.

    DatagramInputFile din;
    if (!din.open(fullpath)) {
      return nullptr;
    }

    std::string head;
    if (!din.read_header(head, _bam_header.size())) {
      return nullptr;
    }

    if (head != _bam_header) {
      return nullptr;
    }

    BamReader reader(&din);
    if (!reader.init()) {
      return nullptr;
    }

    TypedWritable *obj = reader.read_object();

    if (obj == nullptr || !reader.resolve()) {
      return nullptr;
    }

    if (!obj->is_of_type(MaterialBase::get_class_type())) {
      material_cat.error()
        << "File " << fullpath << " does not contain a material!\n";
      return nullptr;
    }

    material = DCAST(MaterialBase, obj);

  } else {
    material_cat.error()
      << "Unsupported material file extension: " << fullpath << "\n";
    return nullptr;
  }

  nassertr(material != nullptr, nullptr);

  material->set_filename(filename);
  material->set_fullpath(fullpath);

  _materials[filename] = material;
  _fullpath_materials[fullpath] = material;

  return material;
}

/**
 *
 */
void MaterialPool::
ns_release_all_materials() {
  _materials.clear();
  _fullpath_materials.clear();
}

/**
 *
 */
MaterialBase *MaterialPool::
ns_find_material(const Filename &filename) const {
  Materials::const_iterator it;

  it = _materials.find(filename);
  if (it != _materials.end()) {
    return (*it).second;
  }

  it = _fullpath_materials.find(filename);
  if (it != _fullpath_materials.end()) {
    return (*it).second;
  }

  return nullptr;
}
