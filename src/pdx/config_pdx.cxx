/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pdx.cxx
 * @author brian
 * @date 2021-06-10
 */

#include "config_pdx.h"

ConfigureDef(config_pdx);
ConfigureFn(config_pdx) {
  init_libpdx();
}

NotifyCategoryDef(pdx, "");

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpdx() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;
}
