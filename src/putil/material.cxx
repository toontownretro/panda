/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.cxx
 * @author lachbr
 * @date 2020-10-13
 */

#include "material.h"
#include "lightMutexHolder.h"
#include "virtualFileSystem.h"
#include "keyValues.h"
#include "string_utils.h"

Material::ScriptCache Material::_cache;
LightMutex Material::_mutex;

/**
 *
 */
int MatTexture::
compare_to(const MatTexture *other) const {
  if (_source != other->_source) {
    return (_source < other->_source) ? -1 : 1;
  }

  int cmp;

  if (_source == S_filename) {
    cmp = _filename.compare_to(other->_filename);
  } else {
    cmp = _name.compare(other->_name);
  }

  if (cmp != 0) {
    return cmp;
  }

  cmp = _stage_name.compare(other->_stage_name);
  if (cmp != 0) {
    return cmp;
  }

  cmp = _texcoord_name.compare(other->_texcoord_name);
  if (cmp != 0) {
    return cmp;
  }

  if (_transform_flags != other->_transform_flags) {
    return (_transform_flags < other->_transform_flags) ? -1 : 1;
  }

  if (has_pos2d()) {
    cmp = _pos.compare_to(other->_pos);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_hpr2d()) {
    cmp = _hpr.compare_to(other->_hpr);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_scale2d()) {
    cmp = _scale.compare_to(other->_scale);
    if (cmp != 0) {
      return cmp;
    }
  }

  return 0;
}

/**
 *
 */
int Material::Bin::
compare_to(const Material::Bin &other) const {
  if (_sort != other._sort) {
    return (_sort < other._sort) ? -1 : 1;
  }

  return _name.compare(other._name);
}

/**
 *
 */
int Material::AlphaTest::
compare_to(const Material::AlphaTest &other) const {
  if (_reference != other._reference) {
    return (_reference < other._reference) ? -1 : 1;
  }

  if (_compare != other._compare) {
    return (_compare < other._compare) ? -1 : 1;
  }

  return 0;
}

/**
 *
 */
Material::
Material() {
  clear();
}

/**
 * Resets the described state.
 */
void Material::
clear() {
  _flags = S_none;

  _fog_off = false;
  _light_off = false;
  _enable_z_write = true;
  _enable_z_test = true;
  _z_offset = 0;
  _color_type = CT_none;
  _color.set(1, 1, 1, 1);
  _color_scale.set(1, 1, 1, 1);
  _color_write = CC_all;
  _cull_face = CFM_clockwise;
  _shader.clear();
  _bin.clear();
  _alpha_test.clear();
  _transparency = TM_unspecified;
  _textures.clear();
  _color_blend = CBM_none;
}

/**
 * Resolves filename references along the model path.
 *
 * Returns true if all filenames were successfully resolved, false otherwise.
 */
bool Material::
resolve_filenames() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  DSearchPath search_path = get_model_path().get_value();
  search_path.append_directory(_fullpath.get_dirname());

  bool success = true;

  // Currently the only things that can use filenames are textures.
  for (size_t i = 0; i < _textures.size(); i++) {
    MatTexture *tex = _textures[i];
    if (tex->get_source() == MatTexture::S_filename) {
      if (!vfs->resolve_filename(tex->_fullpath, search_path)) {
        success = false;
      }
    }
  }

  return success;
}

/**
 * Compares this Material to the indicated other Material.
 *
 * Returns 0 if they are equal, -1 if this object is less than other, or 1 if
 * this object is greater than other.
 */
int Material::
compare_to(const Material *other) const {
  if (_flags != other->_flags) {
    return (_flags < other->_flags) ? -1 : 1;
  }

  if (has_fog_off() && _fog_off != other->_fog_off) {
    return (_fog_off < other->_fog_off) ? -1 : 1;
  }

  if (has_light_off() && _light_off != other->_light_off) {
    return (_light_off < other->_light_off) ? -1 : 1;
  }

  if (has_z_write() && _enable_z_write != other->_enable_z_write) {
    return (_enable_z_write < other->_enable_z_write) ? -1 : 1;
  }

  if (has_z_test() && _enable_z_test != other->_enable_z_test) {
    return (_enable_z_test < other->_enable_z_test) ? -1 : 1;
  }

  if (has_z_offset() && _z_offset != other->_z_offset) {
    return (_z_offset < other->_z_offset) ? -1 : 1;
  }

  int cmp;

  if (has_color()) {
    cmp = _color.compare_to(other->_color);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_color_scale()) {
    cmp = _color_scale.compare_to(other->_color_scale);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_color_write() && _color_write != other->_color_write) {
    return (_color_write < other->_color_write) ? -1 : 1;
  }

  if (has_cull_face() && _cull_face != other->_cull_face) {
    return (_cull_face < other->_cull_face) ? -1 : 1;
  }

  if (has_shader()) {
    cmp = _shader.compare(other->_shader);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_bin()) {
    cmp = _bin.compare_to(other->_bin);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_alpha_test()) {
    cmp = _alpha_test.compare_to(other->_alpha_test);
    if (cmp != 0) {
      return cmp;
    }
  }

  if (has_transparency() && _transparency != other->_transparency) {
    return (_transparency < other->_transparency) ? -1 : 1;
  }

  if (has_textures()) {
    if (_textures.size() != other->_textures.size()) {
      return (_textures.size() < other->_textures.size()) ? -1 : 1;
    }

    for (size_t i = 0; i < _textures.size(); i++) {
      cmp = _textures[i]->compare_to(other->_textures[i]);
      if (cmp != 0) {
        return cmp;
      }
    }
  }

  if (_parameters.size() != other->_parameters.size()) {
    return (_parameters.size() < other->_parameters.size()) ? -1 : 1;
  }

  auto it = _parameters.begin();
  auto it2 = other->_parameters.begin();

  for (; it != _parameters.end() && it2 != other->_parameters.end(); ++it, ++it2) {
    cmp = (*it).first.compare((*it2).first);
    if (cmp != 0) {
      return cmp;
    }

    cmp = (*it).second.compare((*it2).second);
    if (cmp != 0) {
      return cmp;
    }
  }

  //if (has_color_blend() && _color_blend != other->_color_blend) {
  //  return (_color_blend < other->_color_blend) ? -1 : 1;
  //}

  return 0;
}

/**
 * Composes this state script with the indicated state script in place.  States
 * not present on this script but present on the other script will be added to
 * this script.  If a state is present on both this script and the other
 * script, the state from the other script will replace the state on this
 * script.
 */
void Material::
compose(const Material *other) {
  if (other->has_fog_off()) {
    set_fog_off(other->get_fog_off());
  }

  if (other->has_light_off()) {
    set_light_off(other->get_light_off());
  }

  if (other->has_z_write()) {
    set_z_write(other->get_z_write());
  }

  if (other->has_z_test()) {
    set_z_test(other->get_z_test());
  }

  if (other->has_z_offset()) {
    set_z_offset(other->get_z_offset());
  }

  if (other->has_color()) {
    if (other->get_color_type() == CT_flat) {
      set_color(other->get_color());
    } else {
      set_vertex_color();
    }
  }

  if (other->has_color_scale()) {
    set_color_scale(other->get_color_scale());
  }

  if (other->has_color_write()) {
    set_color_write(other->get_color_write());
  }

  if (other->has_cull_face()) {
    set_cull_face(other->get_cull_face());
  }

  if (other->has_shader()) {
    set_shader(other->get_shader());
  }

  for (auto it = other->_parameters.begin();
        it != other->_parameters.end(); ++it) {
    set_param((*it).first, (*it).second);
  }

  if (other->has_bin()) {
    set_bin(other->get_bin_name(), other->get_bin_sort());
  }

  if (other->has_alpha_test()) {
    set_alpha_test(other->get_alpha_test_reference(),
                   other->get_alpha_test_compare());
  }

  if (other->has_transparency()) {
    set_transparency(other->get_transparency());
  }

  if (other->has_textures()) {
    for (size_t i = 0; i < other->get_num_textures(); i++) {
      PT(MatTexture) tex = new MatTexture(*(other->get_texture(i)));
      add_texture(tex);
    }
  }
}

/**
 * Loads a render state script from disk and generates a RenderState.
 */
PT(Material) Material::
load(const Filename &filename, const DSearchPath &search_path) {
  // Find it in the cache
  {
    LightMutexHolder holder(_mutex);
    auto itr = _cache.find(filename);
    if (itr != _cache.end()) {
      return (*itr).second;
    }
  }

  // Not in the cache, read from disk and generate RenderState.

  Filename resolved = filename;
  if (resolved.get_extension().empty()) {
    resolved = resolved.get_fullpath() + get_extension();
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(resolved, search_path)) {
    util_cat.error()
      << "Couldn't find material " << filename.get_fullpath()
      << " on search path " << search_path << "\n";
    return nullptr;
  }

  PT(Material) script;
  {
    util_cat.info()
      << "Loading material " << resolved.get_fullpath() << "\n";

    // This is a text render state script, parse the keyvalues.
    std::string data = vfs->read_file(resolved, true);

    // Append this script's directory to the search path for #includes.
    DSearchPath my_search_path = search_path;
    my_search_path.append_directory(resolved.get_dirname());
    script = parse(data, my_search_path);
    script->set_filename(filename);
    script->set_fullpath(resolved);
    script->resolve_filenames();
  }

  {
    LightMutexHolder holder(_mutex);
    _cache[filename] = script;
  }

  return script;
}

/**
 * Parses the render state script data and generates a RenderState.
 */
PT(Material) Material::
parse(const std::string &data, const DSearchPath &search_path) {
  PT(KeyValues) mat_data = KeyValues::from_string(data);

  PT(Material) script = new Material;

  // Parse the flat parameters.
  for (size_t i = 0; i < mat_data->get_num_keys(); i++) {
    const std::string &key = mat_data->get_key(i);
    const std::string &value = mat_data->get_value(i);

    if (key == "color") {
      script->set_color(KeyValues::to_4f(value));

    } else if (key == "color_scale") {
      script->set_color_scale(KeyValues::to_4f(value));

    } else if (key == "alpha_scale") {
      float scale = atof(value.c_str());

      if (!script->has_color_scale()) {
        script->set_color_scale(LColor(1.0f, 1.0f, 1.0f, scale));
      } else {
        const LVecBase4 &curr_scale = script->get_color_scale();
        script->set_color_scale(
          LVecBase4(curr_scale[0], curr_scale[1], curr_scale[2], scale));
      }

    } else if (key == "z_write") {
      bool write = parse_bool_string(value);
      script->set_z_write(write);

    } else if (key == "z_test") {
      bool test = parse_bool_string(value);
      script->set_z_test(test);

    } else if (key == "no_z") {
      // Shortcut for depthwrite 0 and depthtest 0
      bool noz = parse_bool_string(value);
      if (noz) {
        script->set_z_write(false);
        script->set_z_test(false);
      }

    } else if (key == "z_offset") {
      int offset = atoi(value.c_str());
      script->set_z_offset(offset);

    } else if (key == "no_fog") {
      bool nofog = parse_bool_string(value);
      script->set_fog_off(nofog);

    } else if (key == "no_light") {
      bool nolight = parse_bool_string(value);
      script->set_light_off(nolight);

    } else if (key == "transparency") {
      TransparencyMode mode = TM_unspecified;
      if (is_true_string(value) || value == "alpha") {
        mode = TM_alpha;
      } else if (value == "2" || value == "multisample") {
        mode = TM_multisample;
      } else if (value == "3" || value == "binary") {
        mode = TM_binary;
      } else if (value == "4" || value == "dual") {
        mode = TM_dual;
      } else if (!parse_bool_string(value)) {
        mode = TM_none;
      }
      script->set_transparency(mode);

    } else if (key == "color_write") {
      parse_color_write(value, script);

    } else if (key == "cull") {
      bool enable = parse_bool_string(value);
      CullFaceMode mode = CFM_none;
      if (enable) {
        if (is_true_string(value) || value == "clockwise" || value == "cw") {
          mode = CFM_clockwise;
        } else if (value == "counter_clockwise" || value == "ccw" ||
                   value == "2") {
          mode = CFM_counter_clockwise;
        }
      }

      script->set_cull_face(mode);

    } else if (key == "two_sided") {
      // Alias for "cull none".
      bool two_sided = parse_bool_string(value);
      if (two_sided) {
        script->set_cull_face(CFM_none);
      }

    } else if (key == "shader") {
      script->set_shader(value);

    } else if (key == "#include") {
      // We want to include another state script.
      // We will compose our script with the included script.
      Filename include_filename = Filename::from_os_specific(value);
      PT(Material) include_script = Material::load(include_filename, search_path);
      script->compose(include_script);

    } else {
      // Doesn't match any built-in parameters.
      script->set_param(key, value);
    }
  }

  // Now parse nested blocks inside the state block
  // (for attribs that need multiple parameters, ie textures)
  for (size_t i = 0; i < mat_data->get_num_children(); i++) {
    KeyValues *child = mat_data->get_child(i);
    const std::string &name = child->get_name();
    if (name == "texture") {
      parse_texture_block(child, script);

    } else if (name == "bin") {
      parse_bin_block(child, script);

    } else if (name == "alpha_test") {
      parse_alpha_test_block(child, script);

    } else if (name == "render_mode") {
      parse_render_mode_block(child, script);
    }
  }

  return script;
}

/**
 * Write the indicated RenderState to a script file on disk.
 */
void Material::
write(const Filename &filename, Material::PathMode path_mode) {

  Filename save_dir = filename.get_dirname();
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // We are writing a raw text render state script.

  PT(KeyValues) script = new KeyValues;

  if (has_shader()) {
    script->set_key_value("shader", get_shader());
  }

  if (has_color()) {
    script->set_key_value("color", KeyValues::to_string(get_color()));
  }

  if (has_color_scale()) {
    script->set_key_value("color_scale", KeyValues::to_string(get_color_scale()));
  }

  if (has_z_write()) {
    script->set_key_value("z_write", get_z_write() ? "1" : "0");
  }

  if (has_z_test()) {
    script->set_key_value("z_test", get_z_test() ? "1" : "0");
  }

  if (has_z_offset()) {
    script->set_key_value("z_offset", KeyValues::to_string(get_z_offset()));
  }

  if (has_fog_off() && get_fog_off()) {
    script->set_key_value("no_fog", "1");
  }

  if (has_light_off() && get_light_off()) {
    script->set_key_value("no_light", "1");
  }

  if (has_transparency()) {
    switch (get_transparency()) {
    default:
    case TM_unspecified:
      break;
    case TM_none:
      script->set_key_value("transparency", "off");
      break;
    case TM_alpha:
      script->set_key_value("transparency", "alpha");
      break;
    case TM_multisample:
      script->set_key_value("transparency", "multisample");
      break;
    case TM_binary:
      script->set_key_value("transparency", "binary");
      break;
    case TM_dual:
      script->set_key_value("transparency", "dual");
      break;
    }
  }

  if (has_color_write()) {
    unsigned int channels = get_color_write();
    if (channels == CC_off) {
      script->set_key_value("color_write", "off");
    } else if (channels == CC_all) {
      script->set_key_value("color_write", "all");
    } else {
      std::string channels_str;
      if (channels & CC_red) {
        channels_str += 'r';
      }
      if (channels & CC_green) {
        channels_str += 'g';
      }
      if (channels & CC_blue) {
        channels_str += 'b';
      }
      if (channels & CC_alpha) {
        channels_str += 'a';
      }
      script->set_key_value("color_write", channels_str);
    }
  }

  if (has_cull_face()) {
    CullFaceMode mode = get_cull_face();
    switch (mode) {
    case CFM_none:
      script->set_key_value("cull", "none");
      break;
    case CFM_clockwise:
    default:
      script->set_key_value("cull", "cw");
      break;
    case CFM_counter_clockwise:
      script->set_key_value("cull", "ccw");
      break;
    }
  }

  if (has_textures()) {
    for (size_t i = 0; i < get_num_textures(); i++) {
      MatTexture *tex = get_texture(i);

      PT(KeyValues) tex_block = new KeyValues("texture", script);
      if (!tex->_stage_name.empty()) {
        tex_block->set_key_value("stage", tex->_stage_name);
      }

      if (tex->_source == MatTexture::S_filename) {
        Filename tex_filename = tex->_filename;

        switch (path_mode) {
        case PM_unchanged:
          break;
        case PM_relative:
          tex_filename = _fullpath;
          save_dir.make_absolute(vfs->get_cwd());
          if (!tex_filename.make_relative_to(save_dir, true)) {
            tex_filename.find_on_searchpath(get_model_path());
          }
          break;
        case PM_absolute:
          tex_filename = tex->_fullpath;
          break;
        }

        tex_block->set_key_value("filename", tex_filename);

      } else {
        tex_block->set_key_value("name", tex->_name);
      }

      // If the stage has a specific texcoord name assigned, write that out.
      std::string texcoord_name = tex->_texcoord_name;
      if (!texcoord_name.empty()) {
        tex_block->set_key_value("texcoord", texcoord_name);
      }

      // Write out texture transform if we have it.
      if (tex->has_pos2d()) {
        tex_block->set_key_value("pos", KeyValues::to_string(LCAST(float, tex->get_pos2d())));
      }

      if (tex->has_hpr2d()) {
        tex_block->set_key_value("hpr", KeyValues::to_string(LCAST(float, tex->get_hpr2d())));
      }

      if (tex->has_scale2d()) {
        tex_block->set_key_value("scale", KeyValues::to_string(LCAST(float, tex->get_scale2d())));
      }
    }
  }

  if (has_bin()) {
    PT(KeyValues) cba_block = new KeyValues("bin", script);
    cba_block->set_key_value("name", get_bin_name());
    cba_block->set_key_value("sort", KeyValues::to_string(get_bin_sort()));
  }

  if (has_alpha_test()) {
    PT(KeyValues) ata_block = new KeyValues("alpha_test", script);
    ata_block->set_key_value("reference", KeyValues::to_string(
      get_alpha_test_reference()));
    switch (get_alpha_test_compare()) {
    case AlphaTest::C_never:
      ata_block->set_key_value("compare", "never");
      break;
    case AlphaTest::C_less:
      ata_block->set_key_value("compare", "less");
      break;
    case AlphaTest::C_equal:
      ata_block->set_key_value("compare", "equal");
      break;
    case AlphaTest::C_less_equal:
    default:
      ata_block->set_key_value("compare", "less_equal");
      break;
    case AlphaTest::C_greater:
      ata_block->set_key_value("compare", "greater");
      break;
    case AlphaTest::C_greater_equal:
      ata_block->set_key_value("compare", "greater_equal");
      break;
    case AlphaTest::C_always:
      ata_block->set_key_value("compare", "always");
    }
  }

#if 0
  const RenderModeAttrib *rma;
  if (state->get_attrib(rma)) {
    PT(KeyValues) rma_block = new KeyValues("render_mode", script);
    RenderModeAttrib::Mode mode = rma->get_mode();
    switch (mode) {
    case RenderModeAttrib::M_filled:
      rma_block->set_key_value("mode", "filled");
      break;
    case RenderModeAttrib::M_wireframe:
      rma_block->set_key_value("mode", "wireframe");
      break;
    case RenderModeAttrib::M_filled_wireframe:
      rma_block->set_key_value("mode", "filled_wireframe");
      break;
    case RenderModeAttrib::M_filled_flat:
      rma_block->set_key_value("mode", "filled_flat");
      break;
    case RenderModeAttrib::M_point:
      rma_block->set_key_value("mode", "point");
    }

    rma_block->set_key_value("perspective", KeyValues::to_string(rma->get_perspective()));
    rma_block->set_key_value("wireframe_color", KeyValues::to_string(rma->get_wireframe_color()));
    rma_block->set_key_value("thickness", KeyValues::to_string(rma->get_thickness()));
  }
#endif

  for (auto it = _parameters.begin();
        it != _parameters.end(); ++it) {
    script->set_key_value((*it).first, (*it).second);
  }

  util_cat.info()
    << "Writing material " << filename.get_fullpath() << "\n";
  script->write(filename, 2);
}

/**
 * Parses a string and returns a boolean value based on the contents of the
 * string.  "0", "off", "no", "false", and "none" returns false, anything else
 * returns true.
 */
bool Material::
parse_bool_string(const std::string &value) {
  if (value == "0" || value == "off" || value == "no" || value == "false" ||
      value == "none") {
    return false;
  }

  return true;
}

/**
 * Returns true if the string is equal to "1", "yes", "on", or "true".
 */
bool Material::
is_true_string(const std::string &value) {
  if (value == "1" || value == "on" || value == "yes" || value == "true") {
    return true;
  }

  return false;
}

/**
 * Parses a "texture" block and applies an appropriate TextureAttrib onto the
 * state.  For multiple "texture" blocks, this adds new TextureStages onto the
 * existing TextureAttrib.
 */
void Material::
parse_texture_block(KeyValues *block, Material *script) {
  Filename filename;
  std::string stage_name;
  std::string texcoord_name;
  std::string tex_name;
  LPoint2f pos(0, 0);
  LVector2f hpr(0, 0);
  LVector2f scale(1, 1);
  bool got_pos = false;
  bool got_hpr = false;
  bool got_scale = false;

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key == "stage") {
      stage_name = value;

    } else if (key == "texcoord") {
      texcoord_name = value;

    } else if (key == "filename") {
      filename = value;

    } else if (key == "name") {
      tex_name = value;

    } else if (key == "pos") {
      pos = KeyValues::to_2f(value);
      got_pos = true;

    } else if (key == "hpr") {
      hpr = KeyValues::to_2f(value);
      got_hpr = true;

    } else if (key == "scale") {
      scale = KeyValues::to_2f(value);
      got_scale = true;
    }
  }

  PT(MatTexture) tex = new MatTexture;
  if (tex_name.empty() && !filename.empty()) {
    tex->_source = MatTexture::S_filename;
    tex->_filename = filename;
    tex->_fullpath = filename;
  } else {
    tex->_source = MatTexture::S_engine;
    tex->_name = tex_name;
  }

  tex->_stage_name = stage_name;
  tex->_texcoord_name = texcoord_name;
  if (got_pos) {
    tex->set_pos2d(LCAST(double, pos));
  }
  if (got_hpr) {
    tex->set_hpr2d(LCAST(double, hpr));
  }
  if (got_scale) {
    tex->set_scale2d(LCAST(double, scale));
  }

  script->add_texture(tex);
}

/**
 * Parses a "bin" block and applies an appropriate CullBinAttrib onto the
 * state.
 */
void Material::
parse_bin_block(KeyValues *block, Material *script) {
  std::string bin_name = "opaque";
  int sort = 0;

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);
    if (key == "name") {
      bin_name = value;
    } else if (key == "sort") {
      sort = atoi(value.c_str());
    }
  }

  script->set_bin(bin_name, sort);
}

/**
 * Parses an "alpha_test" block and applies an appropriate AlphaTestAttrib onto
 * the state.
 */
void Material::
parse_alpha_test_block(KeyValues *block, Material *script) {
  float ref = 0.5;
  AlphaTest::Compare cmp = AlphaTest::C_greater_equal;

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key == "reference") {
      ref = atof(value.c_str());

    } else if (key == "compare") {
      if (value == "never") {
        cmp = AlphaTest::C_never;

      } else if (value == "less") {
        cmp = AlphaTest::C_less;

      } else if (value == "equal") {
        cmp = AlphaTest::C_equal;

      } else if (value == "less_equal") {
        cmp = AlphaTest::C_less_equal;

      } else if (value == "greater") {
        cmp = AlphaTest::C_greater;

      } else if (value == "greater_equal") {
        cmp = AlphaTest::C_greater_equal;

      } else if (value == "always") {
        cmp = AlphaTest::C_always;
      }
    }
  }

  script->set_alpha_test(ref, cmp);
}

/**
 * Parses a "render_mode" block and applies can appropriate RenderModeAttrib
 * onto the state.
 */
void Material::
parse_render_mode_block(KeyValues *block, Material *script) {
#if 0
  RenderModeAttrib::Mode mode = RenderModeAttrib::M_unchanged;
  float thickness = 1.0;
  bool perspective = false;
  LColorf wireframe_color = LColorf(1, 1, 1, 1);

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key == "mode") {
      if (value == "filled" || value == "1") {
        mode = RenderModeAttrib::M_filled;

      } else if (value == "wireframe" || value == "2") {
        mode = RenderModeAttrib::M_wireframe;

      } else if (value == "point" || value == "3") {
        mode = RenderModeAttrib::M_point;

      } else if (value == "filled_flat" || value == "4") {
        mode = RenderModeAttrib::M_filled_flat;

      } else if (value == "filled_wireframe" || value == "5") {
        mode = RenderModeAttrib::M_filled_wireframe;
      }

    } else if (key == "thickness") {
      thickness = atof(value.c_str());

    } else if (key == "perspective") {
      perspective = parse_bool_string(value);

    } else if (key == "wireframe_color") {
      wireframe_color = KeyValues::to_4f(value);
    }
  }

  state = state->set_attrib(RenderModeAttrib::make(mode, thickness,
                                                   perspective,
                                                   wireframe_color));
#endif
}

/**
 * Parses a "color_blend" block and applies an appropriate ColorBlendAttrib
 * onto the state.
 */
void Material::
parse_color_blend_block(KeyValues *block, Material *script) {
#if 0
  ColorBlendAttrib::Operand a = ColorBlendAttrib::O_fbuffer_color;
  ColorBlendAttrib::Operand b = ColorBlendAttrib::O_incoming_color;
  //ColorBlendAttrib::Mode;
#endif
}

/**
 * Parses a "color_write" value and applies an appropriate ColorWriteAttrib
 * onto the state.
 */
void Material::
parse_color_write(const std::string &value, Material *script) {
  bool enable = parse_bool_string(value);

  int channels = CC_off;

  if (enable) {
    if (is_true_string(value) || value == "all") {
      channels = CC_all;

    } else {
      for (size_t i = 0; i < value.size(); i++) {
        char c = value[i];
        switch (c) {
        case 'r':
          channels |= CC_red;
          break;
        case 'g':
          channels |= CC_green;
          break;
        case 'b':
          channels |= CC_blue;
          break;
        case 'a':
          channels |= CC_alpha;
          break;
        default:
          break;
        }
      }
    }
  }

  script->set_color_write((ColorChannel)channels);
}
