/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderStatePool.h
 * @author lachbr
 * @date 2020-11-17
 */

#ifndef RENDERSTATEPOOL_H
#define RENDERSTATEPOOL_H

#include "config_pgraph.h"
#include "dSearchPath.h"
#include "config_putil.h"
#include "filename.h"
#include "pmap.h"
#include "renderState.h"
#include "lightMutex.h"

/**
 * Interface for loading RenderState objects from files on disk.  Provides a
 * mechanism to unify identical filenames to single RenderState object.
 */
class EXPCL_PANDA_PGRAPH RenderStatePool {
PUBLISHED:
  RenderStatePool();

  static RenderStatePool *get_global_ptr();

  static CPT(RenderState) load_state(
    const Filename &filename,
    const DSearchPath &search_path = get_model_path());

  static void release_all_states();

private:
  CPT(RenderState) ns_load_state(
    const Filename &filename,
    const DSearchPath &search_path);

  void ns_release_all_states();

private:
  typedef pmap<Filename, CPT(RenderState)> Cache;
  Cache _cache;

  LightMutex _lock;

  static RenderStatePool *_global_ptr;
};

#endif // RENDERSTATEPOOL_H
