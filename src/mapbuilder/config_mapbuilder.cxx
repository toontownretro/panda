/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_mapbuilder.cxx
 * @author brian
 * @date 2021-07-05
 */

#include "config_mapbuilder.h"

NotifyCategoryDef(mapbuilder, "");

ConfigureDef(config_mapbuilder);
ConfigureFn(config_mapbuilder) {
  init_libmapbuilder();
}

/**
 *
 */
void
init_libmapbuilder() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;
}
