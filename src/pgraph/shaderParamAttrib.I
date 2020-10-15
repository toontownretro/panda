/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderParamAttrib.I
 * @author lachbr
 * @date 2020-10-15
 */

/**
 *
 */
INLINE ShaderParamAttrib::
ShaderParamAttrib(const std::string &shader_name) {
  _shader_name = shader_name;
}

/**
 *
 */
INLINE ShaderParamAttrib::
ShaderParamAttrib(const ShaderParamAttrib &other) {
  _shader_name = other._shader_name;
  _params = other._params;
}

/**
 * Creates a new ShaderParamAttrib with the given shader name.
 */
INLINE CPT(RenderAttrib) ShaderParamAttrib::
make(const std::string &shader_name) {
  PT(ShaderParamAttrib) attr = new ShaderParamAttrib(shader_name);

  return return_new(attr);
}

/**
 * Returns a new ShaderParamAttrib with a new shader name, but everything else
 * the same.
 */
INLINE CPT(RenderAttrib) ShaderParamAttrib::
set_shader_name(const std::string &shader_name) const {
  PT(ShaderParamAttrib) attr = new ShaderParamAttrib(*this);
  attr->_shader_name = shader_name;
  return return_new(attr);
}

/**
 * Returns a new ShaderParamAttrib with the new parameter set, but everything
 * else the same.
 */
INLINE CPT(RenderAttrib) ShaderParamAttrib::
set_param(const std::string &key, const std::string &value) const {
  PT(ShaderParamAttrib) attr = new ShaderParamAttrib(*this);
  attr->_params[key] = value;
  return return_new(attr);
}

/**
 * Returns the number of key-value shader parameters.
 */
INLINE int ShaderParamAttrib::
get_num_params() const {
  return (int)_params.size();
}

/**
 * Returns the index of the given parameter key, or -1 if it doesn't exist.
 */
INLINE int ShaderParamAttrib::
find_param(const std::string &key) const {
  return _params.find(key);
}

/**
 * Returns true if the given parameter key exists, or false if it does not.
 */
INLINE bool ShaderParamAttrib::
has_param(const std::string &key) const {
  return find_param(key) != -1;
}

/**
 * Returns the shader parameter key at the given index.
 */
INLINE const std::string &ShaderParamAttrib::
get_param_key(int n) const {
  return _params.get_key(n);
}

/**
 * Returns the shader parameter value at the given index.
 */
INLINE const std::string &ShaderParamAttrib::
get_param_value(int n) const {
  return _params.get_data(n);
}

/**
 * Returns the shader name.
 */
INLINE const std::string &ShaderParamAttrib::
get_shader_name() const {
  return _shader_name;
}
