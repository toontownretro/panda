/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderSource.cxx
 * @author lachbr
 * @date 2020-10-18
 */

#include "shaderSource.h"
#include "virtualFileSystem.h"
#include "config_putil.h"

ShaderSource::SourceCache ShaderSource::_cache;
ShaderSource::RawSourceCache ShaderSource::_raw_cache;

/**
 * Returns a ShaderSource object containing the raw source code of the shader
 * loaded from disk from the indicated filename.
 */
CPT(ShaderSource) ShaderSource::
from_filename(const Filename &filename) {
  SourceCache::const_iterator it = _cache.find(filename);
  if (it != _cache.end()) {
    return (*it).second;
  }

  PT(ShaderSource) src = new ShaderSource;
  src->_format = SF_file;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename resolved = filename;
  if (!vfs->resolve_filename(resolved, get_model_path())) {
    shadermgr_cat.error()
      << "Could not find shader " << filename.get_fullpath()
      << " on model path " << get_model_path() << "\n";
    _cache[filename] = nullptr;
    return nullptr;
  }

  src->_source = vfs->read_file(resolved, true);

  size_t end_of_first_line = src->_source.find_first_of('\n');
  src->_before_defines = src->_source.substr(0, end_of_first_line);
  src->_after_defines = src->_source.substr(end_of_first_line);

  _cache[filename] = src;

  return src;
}

/**
 * Returns a new ShaderSource object from the raw source code.
 */
INLINE CPT(ShaderSource) ShaderSource::
from_raw(const std::string &source) {
  RawSourceCache::const_iterator it = _raw_cache.find(source);
  if (it != _raw_cache.end()) {
    return (*it).second;
  }

  PT(ShaderSource) src = new ShaderSource;
  src->_source = source;
  src->_format = SF_raw;

  _raw_cache[source] = src;

  return src;
}
