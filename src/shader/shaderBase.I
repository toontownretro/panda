/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.I
 * @author lachbr
 * @date 2020-10-16
 */

/**
 *
 */
INLINE ShaderBase::ShaderSetup::
ShaderSetup() {
  _inputs.reserve(32);
  clear();
}

/**
 *
 */
INLINE ShaderBase::ShaderSetup::
ShaderSetup(const ShaderBase::ShaderSetup &copy) :
  _language(copy._language),
  _flags(copy._flags),
  _instance_count(copy._instance_count),
  _inputs(copy._inputs),
  _stage_flags(copy._stage_flags) {

  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    _stages[index] = copy._stages[index];

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }
}

/**
 *
 */
INLINE void ShaderBase::ShaderSetup::
clear() {
  _language = Shader::SL_none;
  _flags = 0;
  _instance_count = 0;
  _inputs.clear();

  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    _stages[index].reset();

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }

  _stage_flags = SF_none;
}

/**
 * Returns a hash for the shader setup.
 */
INLINE size_t ShaderBase::ShaderSetup::
get_hash() const {
  size_t hash = 0u;

  hash = integer_hash<uint32_t>::add_hash(hash, _stage_flags.get_word());
  hash = int_hash::add_hash(hash, _flags);
  hash = int_hash::add_hash(hash, _instance_count);

  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    hash = _stages[index].add_hash(hash);

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }

  hash = size_t_hash::add_hash(hash, _inputs.size());
  for (size_t i = 0; i < _inputs.size(); i++) {
    hash = _inputs[i].add_hash(hash);
  }

  return hash;
}

/**
 *
 */
INLINE bool ShaderBase::ShaderSetup::
operator < (const ShaderBase::ShaderSetup &other) const {
  if (_stage_flags != other._stage_flags) {
    return (_stage_flags < other._stage_flags);
  }

  if (_flags != other._flags) {
    return (_flags < other._flags);
  }

  if (_instance_count != other._instance_count) {
    return (_instance_count < other._instance_count);
  }

  if (_inputs.size() != other._inputs.size()) {
    return (_inputs.size() < other._inputs.size());
  }

  for (size_t i = 0; i < _inputs.size(); i++) {
    if (_inputs[i] != other._inputs[i]) {
      return (_inputs[i] < other._inputs[i]);
    }
  }

  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    if (_stages[index] != other._stages[index]) {
      return (_stages[index] < other._stages[index]);
    }

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }

  return false;
}

/**
 *
 */
INLINE bool ShaderBase::ShaderSetup::
operator ==(const ShaderBase::ShaderSetup &other) const {
  if (_stage_flags != other._stage_flags) {
    return false;
  }

  if (_flags != other._flags) {
    return false;
  }

  if (_instance_count != other._instance_count) {
    return false;
  }

  if (_inputs.size() != other._inputs.size()) {
    return false;
  }

  for (size_t i = 0; i < _inputs.size(); i++) {
    if (_inputs[i] != other._inputs[i]) {
      return false;
    }
  }

  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    if (_stages[index] != other._stages[index]) {
      return false;
    }

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }

  return true;
}

/**
 *
 */
INLINE ShaderBase::
ShaderBase(const std::string &name) :
  Namable(name) {
}

/**
 * Sets the filename of the vertex shader.
 */
INLINE void ShaderBase::
set_vertex_shader(const Filename &filename) {
  _setup._stages[S_vertex].set_source_filename(filename);
  _setup._stage_flags |= SF_vertex;
}

/**
 * Sets the source code of the vertex shader.
 */
INLINE void ShaderBase::
set_vertex_shader_source(const std::string &source) {
  _setup._stages[S_vertex].set_source_raw(source);
  _setup._stage_flags |= SF_vertex;
}

/**
 * Adds a #define for the vertex shader.
 */
INLINE void ShaderBase::
set_vertex_shader_define(const std::string &name, const std::string &value) {
  _setup._stages[S_vertex].set_define(name, value);
}

/**
 * Sets the filename of the pixel shader.
 */
INLINE void ShaderBase::
set_pixel_shader(const Filename &filename) {
  _setup._stages[S_pixel].set_source_filename(filename);
  _setup._stage_flags |= SF_pixel;
}

/**
 * Sets the source code of the pixel shader.
 */
INLINE void ShaderBase::
set_pixel_shader_source(const std::string &source) {
  _setup._stages[S_pixel].set_source_raw(source);
  _setup._stage_flags |= SF_pixel;
}

/**
 * Sets a #define for the pixel shader.
 */
INLINE void ShaderBase::
set_pixel_shader_define(const std::string &name, const std::string &value) {
  _setup._stages[S_pixel].set_define(name, value);
}

/**
 * Sets the filename of the geometry shader.
 */
INLINE void ShaderBase::
set_geometry_shader(const Filename &filename) {
  _setup._stages[S_geometry].set_source_filename(filename);
  _setup._stage_flags |= SF_geometry;
}

/**
 * Sets the source code of the geometry shader.
 */
INLINE void ShaderBase::
set_geometry_shader_source(const std::string &source) {
  _setup._stages[S_geometry].set_source_raw(source);
  _setup._stage_flags |= SF_geometry;
}

/**
 * Sets a #define for the geometry shader.
 */
INLINE void ShaderBase::
set_geometry_shader_define(const std::string &name, const std::string &value) {
  _setup._stages[S_geometry].set_define(name, value);
}

/**
 * Sets the filename of the tessellation shader.
 */
INLINE void ShaderBase::
set_tess_shader(const Filename &filename) {
  _setup._stages[S_tess].set_source_filename(filename);
  _setup._stage_flags |= SF_tess;
}

/**
 * Sets the source code of the tessellation shader.
 */
INLINE void ShaderBase::
set_tess_shader_source(const std::string &source) {
  _setup._stages[S_tess].set_source_raw(source);
  _setup._stage_flags |= SF_tess;
}

/**
 * Sets a #define for the tessellation shader.
 */
INLINE void ShaderBase::
set_tess_shader_define(const std::string &name, const std::string &value) {
  _setup._stages[S_tess].set_define(name, value);
}

/**
 * Sets the filename of the tessellation evaluation shader.
 */
INLINE void ShaderBase::
set_tess_eval_shader(const Filename &filename) {
  _setup._stages[S_tess_eval].set_source_filename(filename);
  _setup._stage_flags |= SF_tess_eval;
}

/**
 * Sets the source code of the tessellation evaluation shader.
 */
INLINE void ShaderBase::
set_tess_eval_shader_source(const std::string &source) {
  _setup._stages[S_tess_eval].set_source_raw(source);
  _setup._stage_flags |= SF_tess_eval;
}

/**
 * Sets a #define for the tessellation evaluation shader.
 */
INLINE void ShaderBase::
set_tess_eval_shader_define(const std::string &name, const std::string &value) {
  _setup._stages[S_tess_eval].set_define(name, value);
}

/**
 * Returns the ShaderStage for the given stage type.
 */
INLINE ShaderStage &ShaderBase::
get_stage(ShaderBase::Stage stage) {
  return _setup._stages[stage];
}

/**
 * Returns whether or not this shader contains the stage of the indicated type.
 */
INLINE bool ShaderBase::
has_stage(StageFlags flags) const {
  return (_setup._stage_flags & flags) != 0;
}

/**
 * Sets a uniform input on the shader.
 */
INLINE void ShaderBase::
set_input(const ShaderInput &input) {
  _setup._inputs.push_back(input);
}

/**
 * Sets a uniform input on the shader.
 */
INLINE void ShaderBase::
set_input(ShaderInput &&input) {
  _setup._inputs.push_back(std::move(input));
}

/**
 * Returns the number of uniform inputs.
 */
INLINE size_t ShaderBase::
get_num_inputs() const {
  return _setup._inputs.size();
}

/**
 * Returns the list of uniform inputs.
 */
INLINE const pvector<ShaderInput> &ShaderBase::
get_inputs() const {
  return _setup._inputs;
}

/**
 * Sets the shader flags.
 */
INLINE void ShaderBase::
set_flags(int flags) {
  _setup._flags |= flags;
}

/**
 * Returns the shader flags.
 */
INLINE int ShaderBase::
get_flags() const {
  return _setup._flags;
}

/**
 * Sets the number of instances that the shader should render.
 */
INLINE void ShaderBase::
set_instance_count(int count) {
  _setup._instance_count = count;
}

/**
 * Returns the number of instances that the shader should render.
 */
INLINE int ShaderBase::
get_instance_count() const {
  return _setup._instance_count;
}

/**
 * Sets the language of the shader.
 */
INLINE void ShaderBase::
set_language(Shader::ShaderLanguage language) {
  _setup._language = language;
}

/**
 * Returns the language of the shader.
 */
INLINE Shader::ShaderLanguage ShaderBase::
get_language() const {
  return _setup._language;
}

/**
 * Resets the shader to a clean slate, ready for the state that needs a shader.
 */
INLINE void ShaderBase::
reset() {
  _setup.clear();
}
