/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderStage.h
 * @author lachbr
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
  _combo_values = copy._combo_values;
  _variation_index = copy._variation_index;
}

/**
 * Resets the ShaderStage to a clean slate, ready for the shader that needs to
 * be generated.
 */
INLINE void ShaderStage::
reset() {
  _object = nullptr;
  _specified_combos.clear();
  _combo_values.clear();
  _variation_index = 0;
}

/**
 * Sets the filename of the source code for this shader stage.
 */
INLINE void ShaderStage::
set_source_filename(const Filename &filename) {
  _object = load_shader_object(filename);
  nassertv(_object != nullptr);

  // Set all combo values to 0.
  _combo_values.resize(_object->get_num_combos());
  //memset(_combo_values.data(), 0, sizeof(int) * _combo_values.size());
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
  nassertv(i < _combo_values.size());
#ifndef NDEBUG
  const ShaderObject::Combo &combo = _object->get_combo(i);
  nassertv(value >= combo.min_val && value <= combo.max_val);
#endif
  _combo_values[i] = value;
  _specified_combos.insert(i);
}

/**
 *
 */
INLINE void ShaderStage::
set_combo_value(const InternalName *name, int value) {
  int index = _object->get_combo_index(name);
  nassertv(index >= 0 && index < (int)_combo_values.size());
#ifndef NDEBUG
  const ShaderObject::Combo &combo = _object->get_combo(index);
  nassertv(value >= combo.min_val && value <= combo.max_val);
#endif
  _combo_values[index] = value;
  _specified_combos.insert(index);
}

/**
 * Calculates the ShaderObject variation index from the values of all combos.
 * This determines the actual precompiled ShaderModule to use for the stage.
 */
INLINE void ShaderStage::
calc_variation_index() {
  _variation_index = 0;
  for (auto it = _specified_combos.begin(); it != _specified_combos.end(); ++it) {
    int i = *it;
    const ShaderObject::Combo &combo = _object->get_combo(i);
    _variation_index += combo.scale * _combo_values[i];
  }
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

  return _object->get_permutation(_variation_index);
}
