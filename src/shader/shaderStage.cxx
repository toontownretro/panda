/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.cxx
 * @author lachbr
 * @date 2020-10-18
 */

#include "shaderStage.h"
#include "virtualFileSystem.h"
#include "config_putil.h"
#include "bamReader.h"
#include "datagramInputFile.h"
#include "bamFile.h"
#include "bam.h"
#include "config_shader.h"
#include "lightMutexHolder.h"

ShaderStage::ObjectCache ShaderStage::_object_cache;
LightMutex ShaderStage::_object_cache_lock("object-cache-lock");

/**
 *
 */
const ShaderObject *ShaderStage::
load_shader_object(const Filename &filename) {
  LightMutexHolder holder(_object_cache_lock);

  CPT(ShaderObject) &obj = _object_cache[filename];

  if (!obj.is_null()) {
    return obj;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename resolved = filename;
  if (!vfs->resolve_filename(resolved, get_model_path())) {
    shadermgr_cat.error()
      << "Could not find shader object " << filename << " on model-path " << get_model_path() << "\n";
    return nullptr;
  }

  shadermgr_cat.info()
    << "Reading shader object " << resolved << "\n";

  // Load from disk.
  DatagramInputFile din;
  if (!din.open(resolved)) {
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

  TypedWritable *tw = reader.read_object();

  if (tw == nullptr || !reader.resolve()) {
    return nullptr;
  }

  if (tw->get_type() != ShaderObject::get_class_type()) {
    return nullptr;
  }

  obj = DCAST(ShaderObject, tw);
  return obj;
}

/**
 *
 */
void ShaderStage::
clear_sho_cache() {
  LightMutexHolder holder(_object_cache_lock);
  _object_cache.clear();
}

/**
 *
 */
void ShaderStage::
spew_variation() {
  if (_object == nullptr) {
    std::cout << "No shader for this stage\n";
  }
  std::cout << "Variation index: " << _variation_index << "\n";
  std::cout << _combo_values.size() << " combo values\n";
  std::cout << _object->get_num_combos() << " combos on object\n";
  for (size_t i = 0; i < _combo_values.size(); ++i) {
    const ShaderObject::Combo &combo = _object->get_combo(i);
    std::cout << combo.name->get_name() << " " << combo.min_val << ".." << combo.max_val << ", value " << _combo_values[i] << "\n";
    std::cout << "scale: " << combo.scale << "\n";
  }
}
