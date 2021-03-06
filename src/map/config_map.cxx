/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_map.cxx
 * @author lachbr
 * @date 2020-12-18
 */

#include "config_map.h"
#include "loaderFileTypeRegistry.h"

NotifyCategoryDef(map, "");

ConfigureDef(config_map);
ConfigureFn(config_map) {
  init_libmap();
}

void
init_libmap() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;
}
