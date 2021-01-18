/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.I
 * @author lachbr
 * @date 2020-10-13
 */

/**
 *
 */
INLINE MatTexture::
MatTexture() {
  clear();
}

/**
 *
 */
INLINE MatTexture::
MatTexture(const MatTexture &other) {
  _source = other._source;
  _filename = other._filename;
  _fullpath = other._fullpath;
  _name = other._name;
  _stage_name = other._stage_name;
  _texcoord_name = other._texcoord_name;
  _pos = other._pos;
  _hpr = other._hpr;
  _scale = other._scale;
  _shear = other._shear;
  _has_transform = other._has_transform;
}

/**
 * Resets the texture state.
 */
INLINE void MatTexture::
clear() {
  _source = S_filename;
  _filename = "";
  _name = "";
  _stage_name = "";
  _texcoord_name = "";
  _has_transform = false;
  _pos.set(0, 0, 0);
  _hpr.set(0, 0, 0);
  _scale.set(1, 1, 1);
  _shear.set(0, 0, 0);
}

/**
 * Sets the texture source.
 */
INLINE void MatTexture::
set_source(Source source) {
  _source = source;
}

/**
 * Returns the texture source.
 */
INLINE MatTexture::Source MatTexture::
get_source() const {
  return _source;
}

/**
 * Sets the filename of the texture.
 */
INLINE void MatTexture::
set_filename(const Filename &filename) {
  _filename = filename;
}

/**
 * Returns the filename of the texture.
 */
INLINE const Filename &MatTexture::
get_filename() const {
  return _filename;
}

/**
 * Sets the fullpath of the texture.
 */
INLINE void MatTexture::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

/**
 * Returns the fullpath of the texture.
 */
INLINE const Filename &MatTexture::
get_fullpath() const {
  return _fullpath;
}

/**
 * Sets the name of the texture.  Only necessary for engine textures.
 */
INLINE void MatTexture::
set_name(const std::string &name) {
  _name = name;
}

/**
 * Returns the name of the texture.  Only set for engine textures.
 */
INLINE const std::string &MatTexture::
get_name() const {
  return _name;
}

/**
 * Sets the stage/slot name of the texture.
 */
INLINE void MatTexture::
set_stage_name(const std::string &name) {
  _stage_name = name;
}

/**
 * Returns the stage/slot name of the texture.
 */
INLINE const std::string &MatTexture::
get_stage_name() const {
  return _stage_name;
}

/**
 * Sets texcoord name of the texture.
 */
INLINE void MatTexture::
set_texcoord_name(const std::string &name) {
  _texcoord_name = name;
}

/**
 * Returns the texcoord name of the texture.
 */
INLINE const std::string &MatTexture::
get_texcoord_name() const {
  return _texcoord_name;
}

/**
 * Sets the texcoord offset position.
 */
INLINE void MatTexture::
set_pos(const LPoint3 &pos) {
  _pos = pos;
}

/**
 * Returns the texcoord offset position.
 */
INLINE const LPoint3 &MatTexture::
get_pos() const {
  return _pos;
}

/**
 * Sets the texcoord offset angles.
 */
INLINE void MatTexture::
set_hpr(const LVector3 &hpr) {
  _hpr = hpr;
}

/**
 * Returns the texcoord offset angles.
 */
INLINE const LVector3 &MatTexture::
get_hpr() const {
  return _hpr;
}

/**
 * Sets the texcoord offset scale.
 */
INLINE void MatTexture::
set_scale(const LVector3 &scale) {
  _scale = scale;
}

/**
 * Returns the texcoord offset scale.
 */
INLINE const LVector3 &MatTexture::
get_scale() const {
  return _scale;
}

/**
 * Sets the texcoord offset shear.
 */
INLINE void MatTexture::
set_shear(const LVector3 &shear) {
  _shear = shear;
}

/**
 * Returns the texcoord offset shear.
 */
INLINE const LVector3 &MatTexture::
get_shear() const {
  return _shear;
}


/**
 * Sets the texcoord offset transform.
 */
INLINE void MatTexture::
set_transform(const LMatrix4 &transform) {
  decompose_matrix(transform, _scale, _shear, _hpr, _pos);
}

/**
 * Returns the texcoord offset transform.
 */
INLINE LMatrix4 MatTexture::
get_transform() const {
  LMatrix4 mat;
  compose_matrix(mat, _scale, _shear, _hpr, _pos);
  return mat;
}

/**
 * Returns true if the texture has a non-identity texcoord transform, false
 * otherwise.
 */
INLINE bool MatTexture::
has_transform() const {
  return _has_transform;
}

/**
 *
 */
INLINE Material::Bin::
Bin() {
  clear();
}

/**
 * Resets the bin state.
 */
INLINE void Material::Bin::
clear() {
  _name = "";
  _sort = 0;
}

/**
 *
 */
INLINE Material::AlphaTest::
AlphaTest() {
  clear();
}

/**
 *
 */
INLINE void Material::AlphaTest::
clear() {
  _reference = 0.0f;
  _compare = C_none;
}

/**
 * Returns the filename extension for a raw text render state script.
 */
INLINE std::string Material::
get_extension() {
  // MAT = Material
  return "mat";
}

/**
 * Returns the filename extension for a binary render state script.
 */
INLINE std::string Material::
get_binary_extension() {
  // MTO = Material Object
  return "mto";
}

/**
 * Sets the filename that the script was loaded from.
 */
INLINE void Material::
set_filename(const Filename &filename) {
  _filename = filename;
}

/**
 * Returns the filename that the script was loaded from.
 */
INLINE const Filename &Material::
get_filename() const {
  return _filename;
}

/**
 * Returns true if the script has a filename it was loaded from, false
 * otherwise.
 */
INLINE bool Material::
has_filename() const {
  return !_filename.empty();
}

/**
 * Clears the filename that the script was loaded from.
 */
INLINE void Material::
clear_filename() {
  _filename = "";
}

/**
 * Sets the fullpath that the script was loaded from.
 */
INLINE void Material::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

/**
 * Returns the fullpath that the script was loaded from.
 */
INLINE const Filename &Material::
get_fullpath() const {
  return _fullpath;
}

/**
 * Returns true if the script has a fullpath it was loaded from, false
 * otherwise.
 */
INLINE bool Material::
has_fullpath() const {
  return !_fullpath.empty();
}

/**
 * Clears the fullpath that the script was loaded from.
 */
INLINE void Material::
clear_fullpath() {
  _fullpath = "";
}

/**
 * Returns true if the indicated states are specified in the script, false
 * otherwise.
 */
INLINE bool Material::
has_state(Material::States states) const {
  return (_flags & states) != 0;
}

/**
 * Sets whether or not fogging should be disabled.
 */
INLINE void Material::
set_fog_off(bool flag) {
  _fog_off = flag;
  _flags |= S_fog_off;
}

/**
 * Returns whether or not fogging should be disabled.
 */
INLINE bool Material::
get_fog_off() const {
  return _fog_off;
}

/**
 * Returns true if fog off has been specified in the script, false otherwise.
 */
INLINE bool Material::
has_fog_off() const {
  return has_state(S_fog_off);
}

/**
 * Clears the fog off specification from the script.
 */
INLINE void Material::
clear_fog_off() {
  _fog_off = false;
  _flags |= S_fog_off;
}

/**
 * Sets whether or not lighting should be disabled.
 */
INLINE void Material::
set_light_off(bool flag) {
  _light_off = flag;
  _flags |= S_light_off;
}

/**
 * Returns whether or not lighting should be disabled.
 */
INLINE bool Material::
get_light_off() const {
  return _light_off;
}

/**
 * Returns true if light off has been specified in the script, false otherwise.
 */
INLINE bool Material::
has_light_off() const {
  return has_state(S_light_off);
}

/**
 * Clears the light off specification from the script.
 */
INLINE void Material::
clear_light_off() {
  _light_off = false;
  _flags |= S_light_off;
}

/**
 * Sets whether or not the state should enable Z/depth writing.
 */
INLINE void Material::
set_z_write(bool flag) {
  _enable_z_write = flag;
  _flags |= S_z_write;
}

/**
 * Returns true if Z/depth writing is enabled on the state, false otherwise.
 */
INLINE bool Material::
get_z_write() const {
  return _enable_z_write;
}

/**
 * Returns true if the Z/depth write state has been specified in the script,
 * false otherwise.
 */
INLINE bool Material::
has_z_write() const {
  return has_state(S_z_write);
}

/**
 * Clears the Z/depth write specification from the script.
 */
INLINE void Material::
clear_z_write() {
  _enable_z_write = true;
  _flags &= ~S_z_write;
}

/**
 * Sets whether or not the state should enable Z/depth testing.
 */
INLINE void Material::
set_z_test(bool flag) {
  _enable_z_test = flag;
  _flags |= S_z_test;
}

/**
 * Returns true if Z/depth testing is enabled on the state, false otherwise.
 */
INLINE bool Material::
get_z_test() const {
  return _enable_z_test;
}

/**
 * Returns true if the Z/depth test state has been specified in the script,
 * false otherwise.
 */
INLINE bool Material::
has_z_test() const {
  return has_state(S_z_test);
}

/**
 * Clears the Z/depth test specification from the script.
 */
INLINE void Material::
clear_z_test() {
  _enable_z_test = true;
  _flags &= ~S_z_test;
}

/**
 * Sets the Z/depth offset that should be applied to the state.
 */
INLINE void Material::
set_z_offset(int offset) {
  _z_offset = offset;
  _flags |= S_z_offset;
}

/**
 * Returns the Z/depth offset that should be applied to the state.
 */
INLINE int Material::
get_z_offset() const {
  return _z_offset;
}

/**
 * Returns true if the Z/depth offset state has been specified in the script,
 * false otherwise.
 */
INLINE bool Material::
has_z_offset() const {
  return has_state(S_z_offset);
}

/**
 * Clears the Z/depth offset specification from the script.
 */
INLINE void Material::
clear_z_offset() {
  _z_offset = 0;
  _flags &= ~S_z_offset;
}

/**
 * Sets the color that should be applied onto the state.  This replaces vertex
 * coloring.
 */
INLINE void Material::
set_color(const LColor &color) {
  _color_type = CT_flat;
  _color = color;
  _flags |= S_color;
}

/**
 * Enables vertex coloring on the material.  This replaces any flat color that
 * has been specified.
 */
INLINE void Material::
set_vertex_color() {
  _color_type = CT_vertex;
  _flags |= S_color;
}

/**
 * Returns the flat color that should be applied onto the state.
 */
INLINE const LColor &Material::
get_color() const {
  return _color;
}

/**
 * Returns true if a color has been specified in the script, false otherwise.
 */
INLINE bool Material::
has_color() const {
  return has_state(S_color) && (get_color_type() != CT_none);
}

/**
 * Clears the color specification from the script.
 */
INLINE void Material::
clear_color() {
  _color.set(1, 1, 1, 1);
  _color_type = CT_none;
  _flags &= ~S_color;
}

/**
 * Returns the type of color set on the state.  This can be either a flat
 * color, vertex coloring, or none.
 */
INLINE Material::ColorType Material::
get_color_type() const {
  return _color_type;
}

/**
 * Sets the color scale that should be applied onto the state.
 */
INLINE void Material::
set_color_scale(const LColor &scale) {
  _color_scale = scale;
  _flags |= S_color_scale;
}

/**
 * Returns the color scale that should be applied onto the state.
 */
INLINE const LColor &Material::
get_color_scale() const {
  return _color_scale;
}

/**
 * Returns true if a color scale has been specified in the script, false
 * otherwise.
 */
INLINE bool Material::
has_color_scale() const {
  return has_state(S_color_scale);
}

/**
 * Clears the color scale specification from the script.
 */
INLINE void Material::
clear_color_scale() {
  _color_scale.set(1, 1, 1, 1);
  _flags &= ~S_color_scale;
}

/**
 * Sets the color channels that should be written to by geometry rendered using
 * this state.
 */
INLINE void Material::
set_color_write(Material::ColorChannel channels) {
  _color_write = channels;
  _flags |= S_color_write;
}

/**
 * Returns the color channels that should be written to by geometry rendered
 * using this state.
 */
INLINE Material::ColorChannel Material::
get_color_write() const {
  return _color_write;
}

/**
 * Returns true if color write has been specified in the script, false
 * otherwise.
 */
INLINE bool Material::
has_color_write() const {
  return has_state(S_color_write);
}

/**
 * Clears the color write specification from the script.
 */
INLINE void Material::
clear_color_write() {
  _color_write = CC_all;
  _flags &= ~S_color_write;
}

/**
 * Sets the side of a polygon using this state that should be culled.
 */
INLINE void Material::
set_cull_face(Material::CullFaceMode mode) {
  _cull_face = mode;
  _flags |= S_cull_face;
}

/**
 * Returns the side of a polygon using this state that should be culled.
 */
INLINE Material::CullFaceMode Material::
get_cull_face() const {
  return _cull_face;
}

/**
 * Returns true if cull face has been specified in the script, false otherwise.
 */
INLINE bool Material::
has_cull_face() const {
  return has_state(S_cull_face);
}

/**
 * Clears the cull face specification from the script.
 */
INLINE void Material::
clear_cull_face() {
  _cull_face = CFM_clockwise;
  _flags &= ~S_cull_face;
}

/**
 * Sets the name of the shader that should be used to render geometry using
 * this state.
 */
INLINE void Material::
set_shader(const std::string &name) {
  _shader = name;
  _flags |= S_shader;
}

/**
 * Returns the name of the shader that should be used to render geometry using
 * this state.
 */
INLINE const std::string &Material::
get_shader() const {
  return _shader;
}

/**
 * Sets a key-value parameter for the shader that should be used to render
 * geometry using this state.
 */
INLINE void Material::
set_param(const std::string &key, const std::string &value) {
  _parameters[key] = value;
}

/**
 * Returns true if the indicated key exists as one of the specified shader
 * parameters, false otherwise.
 */
INLINE bool Material::
has_param_key(const std::string &key) const {
  auto it = _parameters.find(key);
  return it != _parameters.end();
}

/**
 * Returns the shader parameter value associated with the indicated key, or the
 * empty string if none exists.
 */
INLINE std::string Material::
get_param_value(const std::string &key) const {
  auto it = _parameters.find(key);
  if (it == _parameters.end()) {
    return "";
  }

  return (*it).second;
}

/**
 * Returns true if a shader has been specified in the script, false otherwise.
 */
INLINE bool Material::
has_shader() const {
  return has_state(S_shader);
}

/**
 * Clears the shader specification from the script.
 */
INLINE void Material::
clear_shader() {
  _shader.clear();
  _flags &= ~S_shader;
}

/**
 * Sets the cull bin that geometry using this state should be placed into.
 */
INLINE void Material::
set_bin(const std::string &name, int sort) {
  _bin._name = name;
  _bin._sort = sort;
  _flags |= S_cull_bin;
}

/**
 * Returns the name of the bin set on the state.
 */
INLINE const std::string &Material::
get_bin_name() const {
  return _bin._name;
}

/**
 * Returns the sort within the cull bin for this state.
 */
INLINE int Material::
get_bin_sort() const {
  return _bin._sort;
}

/**
 * Returns true if a cull bin has been specified in the script, false
 * otherwise.
 */
INLINE bool Material::
has_bin() const {
  return has_state(S_cull_bin);
}

/**
 * Clears the cull bin specification from the script.
 */
INLINE void Material::
clear_bin() {
  _bin.clear();
  _flags &= ~S_cull_bin;
}

/**
 * Sets the alpha testing function that should be applied to the state.
 */
INLINE void Material::
set_alpha_test(float reference, Material::AlphaTest::Compare compare) {
  _alpha_test._reference = reference;
  _alpha_test._compare = compare;
  _flags |= S_alpha_test;
}

/**
 * Returns the alpha test reference alpha.
 */
INLINE float Material::
get_alpha_test_reference() const {
  return _alpha_test._reference;
}

/**
 * Returns the alpha test comparison function.
 */
INLINE Material::AlphaTest::Compare Material::
get_alpha_test_compare() const {
  return _alpha_test._compare;
}

/**
 * Returns true if alpha testing has been specified in the script, false
 * otherwise.
 */
INLINE bool Material::
has_alpha_test() const {
  return has_state(S_alpha_test);
}

/**
 * Clears the alpha test specification from the script.
 */
INLINE void Material::
clear_alpha_test() {
  _alpha_test.clear();
  _flags &= ~S_alpha_test;
}

/**
 * Sets the transparency mode that should be applied to the state.
 */
INLINE void Material::
set_transparency(Material::TransparencyMode mode) {
  _transparency = mode;
  _flags |= S_transparency;
}

/**
 * Returns the transparency mode that should be applied to the state.
 */
INLINE Material::TransparencyMode Material::
get_transparency() const {
  return _transparency;
}

/**
 * Returns true if transparency has been specified in the script, false
 * otherwise.
 */
INLINE bool Material::
has_transparency() const {
  return has_state(S_transparency);
}

/**
 * Clears the transparency specification from the script.
 */
INLINE void Material::
clear_transparency() {
  _transparency = TM_none;
  _flags &= ~S_transparency;
}

/**
 * Adds a texture onto the state.
 */
INLINE void Material::
add_texture(MatTexture *tex) {
  size_t index;
  for (index = 0; index < _textures.size(); index++) {
    if (_textures[index]->_stage_name == tex->_stage_name) {
      // Replace the texture with the same stage name.
      _textures[index] = tex;
      return;
    }
  }

  // Don't already have a texture with this stage name, add it.
  _textures.push_back(tex);

  _flags |= S_texture;
}

/**
 * Returns the number of textures on the state.
 */
INLINE size_t Material::
get_num_textures() const {
  return _textures.size();
}

/**
 * Returns true if a texture exists on the state with the indicated stage name,
 * false otherwise.
 */
INLINE bool Material::
has_texture(const std::string &name) const {
  for (size_t i = 0; i < _textures.size(); i++) {
    if (_textures[i]->_stage_name == name) {
      return true;
    }
  }

  return false;
}

/**
 * Returns the texture at the indicated index.
 */
INLINE MatTexture *Material::
get_texture(size_t n) const {
  nassertr(n < _textures.size(), nullptr);

  return _textures[n];
}

/**
 * Returns the texture with the indicated stage name, or nullptr if none
 * exists.
 */
INLINE MatTexture *Material::
get_texture(const std::string &name) const {
  for (size_t i = 0; i < _textures.size(); i++) {
    if (_textures[i]->_stage_name == name) {
      return _textures[i];
    }
  }

  return nullptr;
}

/**
 * Removes the texture at the indicated index.
 */
INLINE void Material::
remove_texture(size_t n) {
  nassertv(n < _textures.size());

  _textures.erase(_textures.begin() + n);

  if (_textures.size() == 0) {
    _flags &= ~S_texture;
  }
}

/**
 * Removes the texture with the indicated stage name.
 */
INLINE void Material::
remove_texture(const std::string &name) {
  Textures::iterator it = _textures.begin();
  for (; it != _textures.end(); ++it) {
    if ((*it)->_stage_name == name) {
      break;
    }
  }

  if (it != _textures.end()) {
    _textures.erase(it);
  }

  if (_textures.size() == 0) {
    _flags &= ~S_texture;
  }
}

/**
 * Returns true if one or more textures have been specified in the script,
 * false otherwise.
 */
INLINE bool Material::
has_textures() const {
  return has_state(S_texture);
}

/**
 * Clears all textures specified in the script.
 */
INLINE void Material::
clear_textures() {
  _textures.clear();
  _flags &= ~S_texture;
}
