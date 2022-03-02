/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandadx11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "pandadx11.h"

#include "config_dxgsg11.h"
#include "wdxGraphicsPipe11.h"

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpandadx11() {
  init_libdxgsg11();
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined
 * by this module.
 */
int
get_pipe_type_pandadx11() {
  return wdxGraphicsPipe11::get_class_type().get_index();
}
