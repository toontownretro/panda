/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsStateGuardian11.I
 * @author brian
 * @date 2022-03-01
 */

/**
 *
 */
INLINE ID3D11Device *DXGraphicsStateGuardian11::
get_device() const {
  return _device;
}

/**
 *
 */
INLINE ID3D11DeviceContext *DXGraphicsStateGuardian11::
get_context() const {
  return _context;
}
