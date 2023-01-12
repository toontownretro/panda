/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialPool.h
 * @author brian
 * @date 2021-03-17
 */

#ifndef MATERIALPOOL_H
#define MATERIALPOOL_H

#include "pandabase.h"
#include "filename.h"
#include "material.h"
#include "pmap.h"
#include "lightMutex.h"

/**
 * This class keeps references to loaded or created materials and can unify
 * identical filenames or materials to the same pointer.
 */
class EXPCL_PANDA_MATERIAL MaterialPool {
private:
  MaterialPool();

PUBLISHED:
  INLINE static MaterialPool *get_global_ptr();

  INLINE static PT(Material) load_material(const Filename &filename, const DSearchPath &search_path = get_model_path());
  INLINE static void release_all_materials();
  INLINE static Material *find_material(const Filename &filename);

private:
  PT(Material) ns_load_material(const Filename &filename, const DSearchPath &search_path);
  void ns_release_all_materials();
  Material *ns_find_material(const Filename &filename) const;

private:
  typedef pmap<Filename, PT(Material)> Materials;
  Materials _materials;
  Materials _fullpath_materials;

  LightMutex _lock;

  static MaterialPool *_global_ptr;
};

#include "materialPool.I"

#endif // MATERIALPOOL_H
