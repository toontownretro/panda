/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderObject.I
 * @author brian
 * @date 2020-12-22
 */

/**
 *
 */
INLINE ShaderObject::Combo::
Combo() {
  name = nullptr;
  min_val = 0;
  max_val = 0;
}

/**
 *
 */
INLINE ShaderObject::
ShaderObject() {
  _stage = ShaderModule::Stage::vertex;
  _lang = Shader::SL_none;
  _total_combos = 0;
}

/**
 * Adds a new combo to the shader object using move semantics.
 */
INLINE void ShaderObject::
add_combo(ShaderObject::Combo &&combo) {
  _combos.push_back(std::move(combo));
  calc_total_combos();
}

/**
 * Copies a new combo to the shader object.
 */
INLINE void ShaderObject::
add_combo(const ShaderObject::Combo &combo) {
  _combos.push_back(combo);
  calc_total_combos();
}

/**
 * Returns true if the shader object contains a combo with the indicated name,
 * false otherwise.
 */
INLINE bool ShaderObject::
has_combo(CPT_InternalName name) const {
  Combo key;
  key.name = name;
  return std::find(_combos.begin(), _combos.end(), key) != _combos.end();
}

/**
 * Returns the combo at the indicated index.
 */
INLINE const ShaderObject::Combo &ShaderObject::
get_combo(size_t n) const {
  assert(n < _combos.size());
  return _combos[n];
}

/**
 * Returns the combo with the indicated name.
 */
INLINE const ShaderObject::Combo &ShaderObject::
get_combo(CPT_InternalName name) const {
  Combo key;
  key.name = name;

  auto it = std::find(_combos.begin(), _combos.end(), key);
  assert(it != _combos.end());
  return (*it);
}

/**
 * Returns the number of combo variables.
 */
INLINE size_t ShaderObject::
get_num_combos() const {
  return _combos.size();
}

/**
 * Returns the index of the Combo with the indicated name, or -1 if no combo
 * exists with such name.
 */
INLINE int ShaderObject::
get_combo_index(const InternalName *name) const {
  auto it = _combos_by_name.find(name);
  if (it != _combos_by_name.end()) {
    return (*it).second;
  }
  return -1;
}

/**
 * Appends a new permutation to the shader object.
 */
INLINE void ShaderObject::
add_permutation(ShaderModule *module) {
  _permutations.push_back(module);
}

/**
 * Sets the permutation at the indicated index.
 */
INLINE void ShaderObject::
set_permutation(size_t n, ShaderModule *module) {
  _permutations[n] = module;
}

/**
 * Resizes the permutations array to have the indicated number of elements.
 */
INLINE void ShaderObject::
resize_permutations(size_t count) {
  _permutations.resize(count);
}

/**
 * Returns the permutation at the indicated index.
 */
INLINE ShaderModule *ShaderObject::
get_permutation(size_t n) const {
  nassertr(n >= 0 && n < _permutations.size(), nullptr);
  return _permutations[n];
}

/**
 * Returns the number of permutations on the shader object.  This should be
 * equal to the total number of unique combinations of combo values.
 */
INLINE size_t ShaderObject::
get_num_permutations() const {
  return _permutations.size();
}

/**
 * Returns the total number of possible unique combinations for each combo
 * variable.
 */
INLINE size_t ShaderObject::
get_total_combos() const {
  return _total_combos;
}

/**
 *
 */
INLINE void ShaderObject::
add_skip_command(SkipCommand &&cmd) {
  _skip_commands.push_back(std::move(cmd));
}

/**
 *
 */
INLINE size_t ShaderObject::
get_num_skip_commands() const {
  return _skip_commands.size();
}

/**
 *
 */
INLINE const ShaderObject::SkipCommand *ShaderObject::
get_skip_command(size_t n) const {
  nassertr(n < _skip_commands.size(), nullptr);
  return &_skip_commands[n];
}

/**
 *
 */
INLINE ShaderObject::VariationBuilder::
VariationBuilder(ShaderObject *obj) {
  reset(obj);
}

/**
 *
 */
INLINE void ShaderObject::VariationBuilder::
reset(ShaderObject *obj) {
  _obj = obj;
  if (_obj != nullptr) {
    _combo_values.resize(obj->get_num_combos());
    for (size_t i = 0; i < _combo_values.size(); ++i) {
      _combo_values[i] = 0;
    }
  }
}

/**
 *
 */
INLINE void ShaderObject::VariationBuilder::
set_combo_value(int n, int value) {
  nassertv(n >= 0 && n < (int)_combo_values.size());
  const ShaderObject::Combo &combo = _obj->get_combo(n);
  nassertv(value >= combo.min_val && value <= combo.max_val);
  _combo_values[n] = value;
}

/**
 *
 */
INLINE void ShaderObject::VariationBuilder::
set_combo_value(const InternalName *combo_name, int value) {
  int n = _obj->get_combo_index(combo_name);
  nassertv(n != -1);
  const ShaderObject::Combo &combo = _obj->get_combo(n);
  nassertv(value >= combo.min_val && value <= combo.max_val);
  _combo_values[n] = value;
}
