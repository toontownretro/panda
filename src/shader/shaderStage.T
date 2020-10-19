/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.T
 * @author lachbr
 * @date 2020-10-18
 */

/**
 * Sets a #define for the shader stage.
 */
template <class T>
INLINE void ShaderStage::
set_define(const std::string &name, const T &value) {
  _defines << "#define " << name << " " << value << "\n";
}
