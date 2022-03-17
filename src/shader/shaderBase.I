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
INLINE ShaderBase::ShaderObjectSetup::
ShaderObjectSetup() {
  clear();
}

/**
 *
 */
INLINE ShaderBase::ShaderObjectSetup::
ShaderObjectSetup(const ShaderObjectSetup &copy) :
  _language(copy._language),
  _stage_flags(copy._stage_flags),
  _spec_constants(copy._spec_constants)
{
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
INLINE void ShaderBase::ShaderObjectSetup::
clear() {
  _language = Shader::SL_none;

  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    _stages[index].reset();

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }

  _stage_flags = SF_none;

  _spec_constants.clear();
}

/**
 *
 */
INLINE size_t ShaderBase::ShaderObjectSetup::
get_hash() const {
  size_t hash = 0u;

  hash = integer_hash<uint32_t>::add_hash(hash, _stage_flags.get_word());
  hash = integer_hash<Shader::ShaderLanguage>::add_hash(hash, _language);
  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    hash = _stages[index].add_hash(hash);

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }
  hash = size_t_hash::add_hash(hash, _spec_constants.size());
  for (auto it = _spec_constants.begin(); it != _spec_constants.end(); ++it) {
    hash = pointer_hash::add_hash(hash, (*it).first);
    hash = integer_hash<unsigned int>::add_hash(hash, (*it).second);
  }

  return hash;
}

/**
 *
 */
INLINE bool ShaderBase::ShaderObjectSetup::
operator < (const ShaderObjectSetup &other) const {
  if (_language != other._language) {
    return (_language < other._language);
  }

  if (_stage_flags != other._stage_flags) {
    return (_stage_flags < other._stage_flags);
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

  if (_spec_constants.size() != other._spec_constants.size()) {
    return (_spec_constants.size() < other._spec_constants.size());
  }

  auto it = _spec_constants.begin();
  auto it2 = other._spec_constants.begin();
  while (it != _spec_constants.end() && it2 != other._spec_constants.end()) {
    if ((*it).first != (*it2).first) {
      return (*it).first < (*it2).first;
    }
    if ((*it).second != (*it2).second) {
      return (*it).second < (*it2).second;
    }
    ++it;
    ++it2;
  }

  return false;
}

/**
 *
 */
INLINE bool ShaderBase::ShaderObjectSetup::
operator == (const ShaderObjectSetup &other) const {
  if (_language != other._language) {
    return false;
  }

  if (_stage_flags != other._stage_flags) {
    return false;
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

  if (_spec_constants.size() != other._spec_constants.size()) {
    return false;
  }

  auto it = _spec_constants.begin();
  auto it2 = other._spec_constants.begin();
  while (it != _spec_constants.end() && it2 != other._spec_constants.end()) {
    if ((*it).first != (*it2).first) {
      return false;
    }
    if ((*it).second != (*it2).second) {
      return false;
    }
    ++it;
    ++it2;
  }

  return true;
}

/**
 * Calculates variation indices for all stages turned on by this setup
 * from the combo values of each turned on stage.
 */
INLINE void ShaderBase::ShaderObjectSetup::
calc_variation_indices() {
  BitMask32 mask = _stage_flags;
  int index = mask.get_lowest_on_bit();
  while (index >= 0) {
    _stages[index].calc_variation_index();

    mask.clear_bit(index);
    index = mask.get_lowest_on_bit();
  }
}

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
  _flags(copy._flags),
  _instance_count(copy._instance_count),
  _inputs(copy._inputs)
{
}

/**
 *
 */
INLINE void ShaderBase::ShaderSetup::
clear() {
  _flags = 0;
  _instance_count = 0;
  _inputs.clear();
}

/**
 * Returns a hash for the shader setup.
 */
INLINE size_t ShaderBase::ShaderSetup::
get_hash() const {
  size_t hash = 0u;

  hash = int_hash::add_hash(hash, _flags);
  hash = int_hash::add_hash(hash, _instance_count);

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

  return false;
}

/**
 *
 */
INLINE bool ShaderBase::ShaderSetup::
operator == (const ShaderBase::ShaderSetup &other) const {
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

  return true;
}

/**
 *
 */
INLINE ShaderBase::
ShaderBase(const std::string &name) :
  Namable(downcase(name)) {
}

/**
 * Sets the filename of the vertex shader.
 */
INLINE void ShaderBase::
set_vertex_shader(const Filename &filename) {
  _obj_setup._stages[S_vertex].set_source_filename(filename);
  _obj_setup._stage_flags |= SF_vertex;
}

/**
 * Sets the value of a vertex shader combo by index.
 */
INLINE void ShaderBase::
set_vertex_shader_combo(size_t n, int value) {
  _obj_setup._stages[S_vertex].set_combo_value(n, value);
}

/**
 * Sets the value of a vertex shader combo by name.
 */
INLINE void ShaderBase::
set_vertex_shader_combo(const InternalName *name, int value) {
  _obj_setup._stages[S_vertex].set_combo_value(name, value);
}

/**
 * Sets the filename of the pixel shader.
 */
INLINE void ShaderBase::
set_pixel_shader(const Filename &filename) {
  _obj_setup._stages[S_pixel].set_source_filename(filename);
  _obj_setup._stage_flags |= SF_pixel;
}

/**
 * Sets the value of a pixel shader combo by index.
 */
INLINE void ShaderBase::
set_pixel_shader_combo(size_t n, int value) {
  _obj_setup._stages[S_pixel].set_combo_value(n, value);
}

/**
 * Sets the value of a pixel shader combo by name.
 */
INLINE void ShaderBase::
set_pixel_shader_combo(const InternalName *name, int value) {
  _obj_setup._stages[S_pixel].set_combo_value(name, value);
}

/**
 * Sets the filename of the geometry shader.
 */
INLINE void ShaderBase::
set_geometry_shader(const Filename &filename) {
  _obj_setup._stages[S_geometry].set_source_filename(filename);
  _obj_setup._stage_flags |= SF_geometry;
}

/**
 * Sets the value of a geometry shader combo by index.
 */
INLINE void ShaderBase::
set_geometry_shader_combo(size_t n, int value) {
  _obj_setup._stages[S_geometry].set_combo_value(n, value);
}

/**
 * Sets the value of a geometry shader combo by name.
 */
INLINE void ShaderBase::
set_geometry_shader_combo(const InternalName *name, int value) {
  _obj_setup._stages[S_geometry].set_combo_value(name, value);
}

/**
 * Sets the filename of the tessellation shader.
 */
INLINE void ShaderBase::
set_tess_shader(const Filename &filename) {
  _obj_setup._stages[S_tess].set_source_filename(filename);
  _obj_setup._stage_flags |= SF_tess;
}

/**
 * Sets the value of a tessellation shader combo by index.
 */
INLINE void ShaderBase::
set_tess_shader_combo(size_t n, int value) {
  _obj_setup._stages[S_tess].set_combo_value(n, value);
}

/**
 * Sets the value of a tessellation shader combo by name.
 */
INLINE void ShaderBase::
set_tess_shader_combo(const InternalName *name, int value) {
  _obj_setup._stages[S_tess].set_combo_value(name, value);
}

/**
 * Sets the filename of the tessellation evaluation shader.
 */
INLINE void ShaderBase::
set_tess_eval_shader(const Filename &filename) {
  _obj_setup._stages[S_tess_eval].set_source_filename(filename);
  _obj_setup._stage_flags |= SF_tess_eval;
}

/**
 * Sets the value of a tessellation evaluation shader combo by index.
 */
INLINE void ShaderBase::
set_tess_eval_shader_combo(size_t n, int value) {
  _obj_setup._stages[S_tess_eval].set_combo_value(n, value);
}

/**
 * Sets the value of a tessellation evaluation shader combo by name.
 */
INLINE void ShaderBase::
set_tess_eval_shader_combo(const InternalName *name, int value) {
  _obj_setup._stages[S_tess_eval].set_combo_value(name, value);
}

/**
 * Returns the ShaderStage for the given stage type.
 */
INLINE ShaderStage &ShaderBase::
get_stage(ShaderBase::Stage stage) {
  return _obj_setup._stages[stage];
}

/**
 * Returns whether or not this shader contains the stage of the indicated type.
 */
INLINE bool ShaderBase::
has_stage(StageFlags flags) const {
  return (_obj_setup._stage_flags & flags) != 0;
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
  _obj_setup._language = language;
}

/**
 * Returns the language of the shader.
 */
INLINE Shader::ShaderLanguage ShaderBase::
get_language() const {
  return _obj_setup._language;
}

/**
 * Resets the shader to a clean slate, ready for the state that needs a shader.
 */
INLINE void ShaderBase::
reset() {
  _setup.clear();
  _obj_setup.clear();
}

/**
 * Adds an alternative name that can be used to refer to this shader.
 */
INLINE void ShaderBase::
add_alias(const std::string &name) {
  _aliases.push_back(downcase(name));
}

/**
 * Returns the number of alternative names for this shader.
 */
INLINE size_t ShaderBase::
get_num_aliases() const {
  return _aliases.size();
}

/**
 * Returns the nth alternative name for this shader.
 */
INLINE const std::string &ShaderBase::
get_alias(size_t n) const {
  return _aliases[n];
}

/**
 *
 */
INLINE void ShaderBase::
set_spec_constant(const InternalName *name, bool value) {
  _obj_setup._spec_constants[name] = (unsigned int)value;
}

/**
 *
 */
INLINE void ShaderBase::
set_spec_constant(const InternalName *name, float value) {
  unsigned int val;
  *((float *)&val) = value;
  _obj_setup._spec_constants[name] = val;
}

/**
 *
 */
INLINE void ShaderBase::
set_spec_constant(const InternalName *name, int value) {
  unsigned int val;
  *((int *)&val) = value;
  _obj_setup._spec_constants[name] = val;
}

/**
 *
 */
INLINE void ShaderBase::
set_spec_constant(const InternalName *name, unsigned int value) {
  _obj_setup._spec_constants[name] = value;
}
