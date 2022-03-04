/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxTextureContext11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxTextureContext11.h"

TypeHandle DXTextureContext11::_type_handle;

/**
 * Creates a shader resource view for this texture so it can be used
 * as inputs to shaders.
 */
void DXTextureContext11::
create_srv() {
  if (_d3d_srv != nullptr) {
    return;
  }

  DXGI_FORMAT format;
}

/**
 * Creates a render target view for this texture so it can be used as a
 * render target output.
 */
void DXTextureContext11::
create_rtv() {
}

/**
 * Creates a depth-stencil view for this texture so it can be used as a
 * depth-stencil output.
 */
void DXTextureContext11::
create_dsv() {
}
