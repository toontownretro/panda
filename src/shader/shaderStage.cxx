/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.cxx
 * @author brian
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
load_shader_object(const Filename &filename, Shader::ShaderLanguage lang, ShaderModule::Stage stage) {
  LightMutexHolder holder(_object_cache_lock);

  CPT(ShaderObject) &obj = _object_cache[filename];

  if (!obj.is_null()) {
    return obj;
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (shader_dynamic_compilation && (filename.get_extension() == "sho" || Filename(filename.get_basename_wo_extension()).get_extension() == "sho")) {
    // Re-wire the filename to point to the source version.
    // For now just assume it's under src/shadersnew.
    Filename source_filename = filename.get_basename_wo_extension();
    if (source_filename.get_extension() == "sho") {
      source_filename = source_filename.get_basename_wo_extension();
    }
    std::string ext = ".glsl";
    if (lang == Shader::SL_HLSL) {
      ext = ".hlsl";
    }
    source_filename = Filename("shadersnew") / source_filename;
    source_filename += ext;
    if (!vfs->resolve_filename(source_filename, get_model_path())) {
      shadermgr_cat.error()
        << "Could not find source version of " << filename << " on model-path "
        << get_model_path() << ".  Searched for " << source_filename << ".  "
        << "Falling back to pre-compiled version.\n";

    } else {
      obj = ShaderObject::read_source(lang, stage, source_filename);
      if (obj != nullptr) {
        return obj;
      }
    }
  }

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
