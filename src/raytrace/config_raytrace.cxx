/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_raytrace.cxx
 * @author lachbr
 * @date 2020-09-21
 */

#include "config_raytrace.h"
#include "rayTraceGeometry.h"
#include "rayTraceTriangleMesh.h"

NotifyCategoryDef(raytrace, "");

ConfigureDef(config_raytrace);

ConfigureFn(config_raytrace) {
  init_libraytrace();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libraytrace() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;

  RayTraceGeometry::init_type();
  RayTraceTriangleMesh::init_type();
}
