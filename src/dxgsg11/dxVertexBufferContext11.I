/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxVertexBufferContext11.I
 * @author brian
 * @date 2022-03-01
 */

/**
 * Returns the D3D11 buffer object that represents the vertex buffer.
 */
INLINE ID3D11Buffer *DXVertexBufferContext11::
get_buffer() const {
  return _vbuffer;
}

/**
 * Returns true if the vertex buffer was created with D3D11_USAGE_IMMUTABLE.
 * If true, the vertex buffer data cannot be written to by the CPU after
 * initial creation, and must be completely recreated if the data is modified.
 */
INLINE bool DXVertexBufferContext11::
is_immutable() const {
  return _immutable;
}
