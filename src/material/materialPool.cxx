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
#include "pdxValue.h"
#include "pdxElement.h"
#include "materialRegistry.h"
#include "datagramInputFile.h"
#include "bam.h"
#include "bamReader.h"
#include "lightMutexHolder.h"

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
PT(Material) MaterialPool::
ns_load_material(const Filename &filename, const DSearchPath &search_path) {
  Materials::const_iterator it;
  {
    LightMutexHolder holder(_lock);
    it = _materials.find(filename);
    if (it != _materials.end()) {
      return (*it).second;
    }
  }

  Filename fullpath = filename;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(fullpath, search_path)) {
    material_cat.error()
      << "Couldn't find material " << filename << " on search path "
      << search_path << "\n";
    return nullptr;
  }

  {
    LightMutexHolder holder(_lock);
    it = _fullpath_materials.find(fullpath);
    if (it != _fullpath_materials.end()) {
      return (*it).second;
    }
  }

  // Not in cache, load it up.

  PT(Material) material;

  material_cat.info()
    << "Loading material " << fullpath << "\n";

  if (fullpath.get_extension() == "pmat") {
    // PDX material file.

    PDXValue pdx_data;
    if (!pdx_data.read(fullpath)) {
      material_cat.error()
        << "Could not load material file " << fullpath << "\n";
      return nullptr;
    }

    if (!pdx_data.is_element()) {
      material_cat.error()
        << "Expected PDXElement in material file " << fullpath << "\n";
      return nullptr;
    }

    PDXElement *mat_data = pdx_data.get_element();

    int material_idx = mat_data->find_attribute("material");
    if (material_idx == -1) {
      material_cat.error()
        << "Material file " << fullpath << " does not specify a material name.\n";
      return nullptr;
    }

    std::string material_name = mat_data->get_attribute_value(material_idx).get_string();

    material = MaterialRegistry::get_global_ptr()->create_material(material_name);
    if (material == nullptr) {
      material_cat.error()
        << "Could not create material by name " << material_name << "\n";
      return nullptr;
    }

    // Append the dirname of the material file to the search path.  This search
    // path will be used to locate textures referenced by the material.
    DSearchPath mat_search_path = search_path;
    mat_search_path.append_directory(fullpath.get_dirname());

    material->read_pdx(mat_data, mat_search_path);

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

    if (!obj->is_of_type(Material::get_class_type())) {
      material_cat.error()
        << "File " << fullpath << " does not contain a material!\n";
      return nullptr;
    }

    material = DCAST(Material, obj);

  } else {
    material_cat.error()
      << "Unsupported material file extension: " << fullpath << "\n";
    return nullptr;
  }

  nassertr(material != nullptr, nullptr);

  material->set_filename(filename);
  material->set_fullpath(fullpath);

  {
    LightMutexHolder holder(_lock);
    _materials[filename] = material;
    _fullpath_materials[fullpath] = material;
  }

  return material;
}

/**
 *
 */
void MaterialPool::
ns_release_all_materials() {
  LightMutexHolder holder(_lock);

  _materials.clear();
  _fullpath_materials.clear();
}

/**
 *
 */
Material *MaterialPool::
ns_find_material(const Filename &filename) const {
  LightMutexHolder holder(_lock);

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
