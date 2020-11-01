/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.T
 * @author lachbr
 * @date 2020-10-18
 */

/**
 * Sets a #define for the vertex shader.
 */
template <class T>
INLINE void ShaderBase::
set_vertex_shader_define(const std::string &name, const T &value) {
  _setup._stages[S_vertex].set_define(name, value);
}

/**
 * Sets the #define for the pixel shader.
 */
template <class T>
INLINE void ShaderBase::
set_pixel_shader_define(const std::string &name, const T &value) {
  _setup._stages[S_pixel].set_define(name, value);
}

/**
 * Sets a #define for the geometry shader.
 */
template <class T>
INLINE void ShaderBase::
set_geometry_shader_define(const std::string &name, const T &value) {
  _setup._stages[S_geometry].set_define(name, value);
}

/**
 * Sets a #define for the tessellation shader.
 */
template <class T>
INLINE void ShaderBase::
set_tess_shader_define(const std::string &name, const T &value) {
  _setup._stages[S_tess].set_define(name, value);
}

/**
 * Sets a #define for the tessellation evaluation shader.
 */
template <class T>
INLINE void ShaderBase::
set_tess_eval_shader_define(const std::string &name, const T &value) {
  _setup._stages[S_tess_eval].set_define(name, value);
}
