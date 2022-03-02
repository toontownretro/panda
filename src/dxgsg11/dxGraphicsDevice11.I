/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsDevice11.I
 * @author brian
 * @date 2022-03-01
 */

/**
 *
 */
INLINE IDXGIAdapter1 *DXGraphicsDevice11::
get_adapter() const {
  return _adapter;
}

/**
 *
 */
INLINE ID3D11Device *DXGraphicsDevice11::
get_device() const {
  return _device;
}

/**
 *
 */
INLINE ID3D11DeviceContext *DXGraphicsDevice11::
get_context() const {
  return _context;
}

/**
 *
 */
INLINE D3D_FEATURE_LEVEL DXGraphicsDevice11::
get_feature_level() const {
  return _feature_level;
}
