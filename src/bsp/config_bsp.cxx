/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_bsp.cxx
 * @author brian
 * @date 2020-12-30
 */

#include "config_bsp.h"
#include "bspData.h"
#include "bspRoot.h"
#include "bspWorld.h"
#include "loaderFileTypeBSP.h"
#include "loaderFileTypeRegistry.h"

ConfigureDef(config_bsp);
ConfigureFn(config_bsp) {
  init_libbsp();
}

NotifyCategoryDef(bsp, "");

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libbsp() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  BSPData::init_type();
  BSPRoot::init_type();
  LoaderFileTypeBSP::init_type();
  BSPWorld::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  reg->register_type(new LoaderFileTypeBSP);
}
