/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialPool.I
 * @author lachbr
 * @date 2021-03-17
 */

/**
 * Returns the global MaterialPool pointer.
 */
INLINE MaterialPool *MaterialPool::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new MaterialPool;
  }

  return _global_ptr;
}

/**
 * Loads the indicated material file from disk and returns a pointer to the
 * loaded material.  If a given filename has already been loaded, returns the
 * same material pointer.
 */
INLINE PT(MaterialBase) MaterialPool::
load_material(const Filename &filename, const DSearchPath &search_path) {
  return get_global_ptr()->ns_load_material(filename, search_path);
}

/**
 * Removes all materials from the filename cache.
 */
INLINE void MaterialPool::
release_all_materials() {
  get_global_ptr()->ns_release_all_materials();
}

/**
 * Returns the material in the cache with the indicated filename, or NULL if
 * there is no such material.
 *
 * Does not actually load the material if it's not in the cache.  Use
 * load_material() to do that.
 */
INLINE MaterialBase *MaterialPool::
find_material(const Filename &filename) {
  return get_global_ptr()->ns_find_material(filename);
}
