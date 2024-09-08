/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.h
 * @author brian
 * @date 2020-10-18
 */

/**
 *
 */
INLINE ShaderStage::
ShaderStage() {
  _object = nullptr;
  _variation_index = 0;
}

/**
 *
 */
INLINE void ShaderStage::
operator = (const ShaderStage &copy) {
  _object = copy._object;
  _builder = copy._builder;
  _variation_index = copy._variation_index;
}

/**
 * Resets the ShaderStage to a clean slate, ready for the shader that needs to
 * be generated.
 */
INLINE void ShaderStage::
reset() {
  _object = nullptr;
  _builder.reset(nullptr);
  _variation_index = 0;
}

/**
 * Sets the filename of the source code for this shader stage.
 */
INLINE void ShaderStage::
set_source_filename(const Filename &filename, Shader::ShaderLanguage lang, ShaderModule::Stage stage) {
  _object = load_shader_object(filename, lang, stage);
  nassertv(_object != nullptr);
  _builder.reset((ShaderObject *)_object);
}

/**
 * Adds the hash of the shader stage onto the running hash.
 */
INLINE size_t ShaderStage::
add_hash(size_t hash) const {
  hash = pointer_hash::add_hash(hash, _object);
  hash = size_t_hash::add_hash(hash, _variation_index);
  return hash;
}

/**
 *
 */
INLINE bool ShaderStage::
operator < (const ShaderStage &other) const {
  if (_object != other._object) {
    return (_object < other._object);
  }

  if (_variation_index != other._variation_index) {
    return (_variation_index < other._variation_index);
  }

  return false;
}

/**
 *
 */
INLINE bool ShaderStage::
operator == (const ShaderStage &other) const {
  if (_object != other._object) {
    return false;
  }

  if (_variation_index != other._variation_index) {
    return false;
  }

  return true;
}

/**
 *
 */
INLINE void ShaderStage::
set_combo_value(size_t i, int value) {
  _builder.set_combo_value(i, value);
}

/**
 *
 */
INLINE void ShaderStage::
set_combo_value(const InternalName *name, int value) {
  _builder.set_combo_value(name, value);
}

/**
 * Calculates the ShaderObject variation index from the values of all combos.
 * This determines the actual precompiled ShaderModule to use for the stage.
 */
INLINE void ShaderStage::
calc_variation_index() {
  _variation_index = _builder.get_module_index();
}

/**
 *
 */
INLINE size_t ShaderStage::
get_variation_index() const {
  return _variation_index;
}

/**
 *
 */
INLINE const ShaderObject *ShaderStage::
get_object() const {
  return _object;
}

/**
 *
 */
INLINE ShaderModule *ShaderStage::
get_module() const {
  if (_object == nullptr) {
    return nullptr;
  }

  return _builder.get_module(shader_dynamic_compilation);
}
