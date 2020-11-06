/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderMangerBase.I
 * @author lachbr
 * @date 2020-11-03
 */

/**
 * Sets the global shader manager.
 */
INLINE void ShaderManagerBase::
set_global_shader_manager(ShaderManagerBase *mgr) {
  _global_mgr = mgr;
}

/**
 * Returns the global shader manager.
 */
INLINE ShaderManagerBase *ShaderManagerBase::
get_global_shader_manager() {
  return _global_mgr;
}
