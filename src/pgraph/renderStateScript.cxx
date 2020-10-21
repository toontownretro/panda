/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderStateScript.cxx
 * @author lachbr
 * @date 2020-10-13
 */

#include "renderStateScript.h"
#include "lightMutexHolder.h"
#include "virtualFileSystem.h"
#include "keyValues.h"

// These are the attribs that can be set through the scripts.
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "cullFaceAttrib.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "depthOffsetAttrib.h"
#include "transparencyAttrib.h"
#include "fogAttrib.h"
#include "internalName.h"
#include "lightAttrib.h"
#include "cullBinAttrib.h"
#include "colorBlendAttrib.h"
#include "colorWriteAttrib.h"
#include "alphaTestAttrib.h"
#include "renderModeAttrib.h"
#include "shaderParamAttrib.h"
#include "textureAttrib.h"
#include "texturePool.h"
#include "textureStage.h"

RenderStateScript::ScriptCache RenderStateScript::_cache;
LightMutex RenderStateScript::_mutex;

/**
 * Loads a render state script from disk and generates a RenderState.
 */
CPT(RenderState) RenderStateScript::
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

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(resolved, search_path, ".pmat")) {
    pgraph_cat.error()
      << "Couldn't find render state script " << filename.get_fullpath()
      << " on search path " << search_path << "\n";
    return RenderState::make_empty();
  }

  pgraph_cat.info()
    << "Loading render state script " << resolved.get_fullpath() << "\n";

  std::string data = vfs->read_file(resolved, true);

  // Append this script's directory to the search path for #includes.
  DSearchPath my_search_path = search_path;
  my_search_path.append_directory(resolved.get_dirname());
  CPT(RenderState) state = parse(data, my_search_path);
  state->_filename = filename;
  state->_fullpath = resolved;

  {
    LightMutexHolder holder(_mutex);
    _cache[filename] = state;
  }

  return state;
}

/**
 * Parses the render state script data and generates a RenderState.
 */
CPT(RenderState) RenderStateScript::
parse(const std::string &data, const DSearchPath &search_path) {
  PT(CKeyValues) mat_data = CKeyValues::from_string(data);

  CPT(RenderState) state = RenderState::make_empty();
  CPT(TextureAttrib) texattr = nullptr;

  // Parse the flat parameters.
  for (size_t i = 0; i < mat_data->get_num_keys(); i++) {
    const std::string &key = mat_data->get_key(i);
    const std::string &value = mat_data->get_value(i);

    if (key == "color") {
      state = state->set_attrib(ColorAttrib::make_flat(CKeyValues::to_4f(value)));

    } else if (key == "color_scale") {
      state = state->set_attrib(ColorScaleAttrib::make(CKeyValues::to_4f(value)));

    } else if (key == "z_write") {
      bool write = parse_bool_string(value);
      state = state->set_attrib(DepthWriteAttrib::make(write ? DepthWriteAttrib::M_on : DepthWriteAttrib::M_off));

    } else if (key == "z_test") {
      bool test = parse_bool_string(value);
      state = state->set_attrib(DepthTestAttrib::make(test ? DepthTestAttrib::M_less_equal : DepthTestAttrib::M_none));

    } else if (key == "no_z") {
      // Shortcut for depthwrite 0 and depthtest 0
      bool noz = parse_bool_string(value);
      if (noz) {
        state = state->set_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
        state = state->set_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
      }

    } else if (key == "z_offset") {
      int offset = atoi(value.c_str());
      state = state->set_attrib(DepthOffsetAttrib::make(offset));

    } else if (key == "no_fog") {
      bool nofog = parse_bool_string(value);
      if (nofog) {
        state = state->set_attrib(FogAttrib::make_off());
      }

    } else if (key == "no_light") {
      bool nolight = parse_bool_string(value);
      if (nolight) {
        state = state->set_attrib(LightAttrib::make_all_off());
      }

    } else if (key == "transparency") {
      TransparencyAttrib::Mode mode = TransparencyAttrib::M_none;
      if (is_true_string(value) || value == "alpha") {
        mode = TransparencyAttrib::M_alpha;
      } else if (value == "2" || value == "premultiplied_alpha") {
        mode = TransparencyAttrib::M_premultiplied_alpha;
      } else if (value == "3" || value == "multisample") {
        mode = TransparencyAttrib::M_multisample;
      } else if (value == "4" || value == "multisample_mask") {
        mode = TransparencyAttrib::M_multisample_mask;
      } else if (value == "5" || value == "binary") {
        mode = TransparencyAttrib::M_binary;
      } else if (value == "6" || value == "dual") {
        mode = TransparencyAttrib::M_dual;
      }
      state = state->set_attrib(TransparencyAttrib::make(mode));

    } else if (key == "color_write") {
      parse_color_write(value, state);

    } else if (key == "cull") {
      bool enable = parse_bool_string(value);
      CullFaceAttrib::Mode mode = CullFaceAttrib::M_cull_none;
      if (enable) {
        if (is_true_string(value) || value == "clockwise" || value == "cw") {
          mode = CullFaceAttrib::M_cull_clockwise;
        } else if (value == "counter_clockwise" || value == "ccw" ||
                   value == "2") {
          mode = CullFaceAttrib::M_cull_counter_clockwise;
        } else if (value == "unchanged") {
          mode = CullFaceAttrib::M_cull_unchanged;
        }
      }

      state = state->set_attrib(CullFaceAttrib::make(mode));

    } else if (key == "two_sided") {
      // Alias for "cull none".
      bool two_sided = parse_bool_string(value);
      if (two_sided) {
        state = state->set_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
      }

    } else if (key == "#include") {
      // We want to include another state script.
      // We will compose our state with the included state.
      Filename include_filename = Filename::from_os_specific(value);
      CPT(RenderState) include_state = RenderStateScript::load(include_filename, search_path);
      state = state->compose(include_state);
    }
  }

  // Now parse nested blocks inside the state block
  // (for attribs that need multiple parameters, ie textures)
  for (size_t i = 0; i < mat_data->get_num_children(); i++) {
    CKeyValues *child = mat_data->get_child(i);
    const std::string &name = child->get_name();
    if (name == "texture") {
      parse_texture_block(child, state);

    } else if (name == "bin") {
      parse_bin_block(child, state);

    } else if (name == "alpha_test") {
      parse_alpha_test_block(child, state);

    } else if (name == "shader") {
      parse_shader_block(child, state);

    } else if (name == "render_mode") {
      parse_render_mode_block(child, state);
    }
  }

  return state;
}

/**
 * Parses a string and returns a boolean value based on the contents of the
 * string.  "0", "off", "no", "false", and "none" returns false, anything else
 * returns true.
 */
bool RenderStateScript::
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
bool RenderStateScript::
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
void RenderStateScript::
parse_texture_block(CKeyValues *block, CPT(RenderState) &state) {
  CPT(RenderAttrib) texattr = state->get_attrib(TextureAttrib::get_class_slot());
  if (!texattr) {
    // We don't already have a TextureAttrib; make one.
    texattr = TextureAttrib::make();
  }

  Filename filename;
  Filename alpha_filename;
  std::string stage_name;
  std::string tex_name;
  bool cubemap = false;

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key == "stage") {
      stage_name = value;

    } else if (key == "filename") {
      filename = value;

    } else if (key == "alpha_filename") {
      alpha_filename = value;

    } else if (key == "cubemap") {
      cubemap = parse_bool_string(value);

    } else if (key == "name") {
      tex_name = value;
    }
  }

  PT(TextureStage) stage = new TextureStage(stage_name);

  PT(Texture) tex;
  if (!filename.empty()) {
    // Load the texture up from disk.
    if (cubemap) {
      tex = TexturePool::load_cube_map(filename);
    } else if (!alpha_filename.empty()) {
      tex = TexturePool::load_texture(filename, alpha_filename);
    } else {
      tex = TexturePool::load_texture(filename);
    }
  } else if (!tex_name.empty()) {
    // We would like to use an engine/application generated texture.
    tex = TexturePool::find_engine_texture(tex_name);
  }

  texattr = DCAST(TextureAttrib, texattr)->add_on_stage(stage, tex);

  state = state->set_attrib(texattr);
}

/**
 * Parses a "bin" block and applies an appropriate CullBinAttrib onto the
 * state.
 */
void RenderStateScript::
parse_bin_block(CKeyValues *block, CPT(RenderState) &state) {
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

  state = state->set_attrib(CullBinAttrib::make(bin_name, sort));
}

/**
 * Parses an "alpha_test" block and applies an appropriate AlphaTestAttrib onto
 * the state.
 */
void RenderStateScript::
parse_alpha_test_block(CKeyValues *block, CPT(RenderState) &state) {
  float ref = 0.5;
  AlphaTestAttrib::PandaCompareFunc cmp = AlphaTestAttrib::M_none;

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key == "reference") {
      ref = atof(value.c_str());

    } else if (key == "compare") {
      if (value == "never") {
        cmp = AlphaTestAttrib::M_never;

      } else if (value == "less") {
        cmp = AlphaTestAttrib::M_less;

      } else if (value == "equal") {
        cmp = AlphaTestAttrib::M_equal;

      } else if (value == "less_equal") {
        cmp = AlphaTestAttrib::M_less_equal;

      } else if (value == "greater") {
        cmp = AlphaTestAttrib::M_greater;

      } else if (value == "greater_equal") {
        cmp = AlphaTestAttrib::M_greater_equal;

      } else if (value == "always") {
        cmp = AlphaTestAttrib::M_always;
      }
    }
  }

  state = state->set_attrib(AlphaTestAttrib::make(cmp, ref));
}

/**
 * Parses a "shader" block and applies an appropriate ShaderParamAttrib onto
 * the state.
 */
void RenderStateScript::
parse_shader_block(CKeyValues *block, CPT(RenderState) &state) {
  std::string name = "default";

  int name_idx = block->find_key("name");
  if (name_idx != -1) {
    name = block->get_value(name_idx);
  }

  CPT(RenderAttrib) spa = ShaderParamAttrib::make(name);

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key != "name") {
      spa = DCAST(ShaderParamAttrib, spa)->set_param(key, value);
    }
  }

  state = state->set_attrib(spa);
}

/**
 * Parses a "render_mode" block and applies can appropriate RenderModeAttrib
 * onto the state.
 */
void RenderStateScript::
parse_render_mode_block(CKeyValues *block, CPT(RenderState) &state) {
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
      wireframe_color = CKeyValues::to_4f(value);
    }
  }

  state = state->set_attrib(RenderModeAttrib::make(mode, thickness,
                                                   perspective,
                                                   wireframe_color));
}

/**
 * Parses a "color_blend" block and applies an appropriate ColorBlendAttrib
 * onto the state.
 */
void RenderStateScript::
parse_color_blend_block(CKeyValues *block, CPT(RenderState) &state) {
  ColorBlendAttrib::Operand a = ColorBlendAttrib::O_fbuffer_color;
  ColorBlendAttrib::Operand b = ColorBlendAttrib::O_incoming_color;
  //ColorBlendAttrib::Mode;
}

/**
 * Parses a "color_write" value and applies an appropriate ColorWriteAttrib
 * onto the state.
 */
void RenderStateScript::
parse_color_write(const std::string &value, CPT(RenderState) &state) {
  bool enable = parse_bool_string(value);

  unsigned int channels = ColorWriteAttrib::C_off;

  if (enable) {
    if (is_true_string(value) || value == "all") {
      channels = ColorWriteAttrib::C_all;

    } else {
      for (size_t i = 0; i < value.size(); i++) {
        char c = value[i];
        switch (c) {
        case 'r':
          channels |= ColorWriteAttrib::C_red;
          break;
        case 'g':
          channels |= ColorWriteAttrib::C_green;
          break;
        case 'b':
          channels |= ColorWriteAttrib::C_blue;
          break;
        case 'a':
          channels |= ColorWriteAttrib::C_alpha;
          break;
        default:
          break;
        }
      }
    }
  }

  state = state->set_attrib(ColorWriteAttrib::make(channels));
}
