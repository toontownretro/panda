/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderStatePool.cxx
 * @author lachbr
 * @date 2020-11-17
 */

#include "renderStatePool.h"
#include "lightMutexHolder.h"
#include "virtualFileSystem.h"

RenderStatePool *RenderStatePool::_global_ptr = nullptr;

/**
 *
 */
RenderStatePool::
RenderStatePool() :
  _lock("RenderStatePool") {
}

/**
 * Returns the global RenderStatePool object.
 */
RenderStatePool *RenderStatePool::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new RenderStatePool;
  }

  return _global_ptr;
}

/**
 * Loads and returns a RenderState object from the given filename.
 */
CPT(RenderState) RenderStatePool::
load_state(const Filename &filename, const DSearchPath &search_path) {
  RenderStatePool *ptr = get_global_ptr();
  return ptr->ns_load_state(filename, search_path);
}

/**
 * Releases all RenderStates from the filename cache.
 */
void RenderStatePool::
release_all_states() {
  RenderStatePool *ptr = get_global_ptr();
  ptr->ns_release_all_states();
}

/**
 * Loads and returns a RenderState object from the given filename.
 */
CPT(RenderState) RenderStatePool::
ns_load_state(const Filename &filename, const DSearchPath &search_path) {
  {
    LightMutexHolder holder(_lock);
    Cache::const_iterator it = _cache.find(filename);
    if (it != _cache.end()) {
      return (*it).second;
    }
  }

  CPT(RenderState) state = RenderState::make(filename, search_path);

  {
    LightMutexHolder holder(_lock);
    _cache[filename] = state;
  }

  return state;
}

/**
 * Releases all RenderStates from the filename cache.
 */
void RenderStatePool::
ns_release_all_states() {
  LightMutexHolder holder(_lock);

  _cache.clear();
}
