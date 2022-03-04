/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleDXBC.I
 * @author brian
 * @date 2022-03-02
 */

/**
 * Returns the ID3DBlob containing the shader byte code.
 */
INLINE ID3DBlob *ShaderModuleDXBC::
get_byte_code() const {
  return _byte_code;
}
