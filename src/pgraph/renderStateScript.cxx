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
#include "bamFile.h"

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
  if (resolved.get_extension().empty()) {
    resolved = resolved.get_fullpath() +
      default_render_state_script_extension.get_value();
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (!vfs->resolve_filename(resolved, search_path)) {
    pgraph_cat.error()
      << "Couldn't find render state script " << filename.get_fullpath()
      << " on search path " << search_path << "\n";
    return RenderState::make_empty();
  }

  CPT(RenderState) state;

  if (resolved.get_extension() == get_binary_extension()) {
    // This is a binary render state script, load up the bam file.

    pgraph_cat.info()
      << "Loading binary render state script " << resolved.get_fullpath()
      << "\n";

    BamFile bam;
    if (!bam.open_read(resolved)) {
      pgraph_cat.error()
        << "Couldn't open binary render state script "
        << resolved.get_fullpath() << "\n";
      return nullptr;
    }

    TypedWritable *obj = bam.read_object();
    if (obj == nullptr || !bam.resolve()) {
      pgraph_cat.error()
        << "Couldn't read binary render state script "
        << resolved.get_fullpath() << "\n";
      bam.close();
      return nullptr;
    }

    if (!obj->is_of_type(RenderState::get_class_type())) {
      pgraph_cat.error()
        << resolved.get_fullpath() << " is not a valid binary render state "
        << "script.\n";
      bam.close();
      return nullptr;
    }

    state = DCAST(RenderState, obj);
    bam.close();

  } else {
    pgraph_cat.info()
      << "Loading render state script " << resolved.get_fullpath() << "\n";

    // This is a text render state script, parse the keyvalues.
    std::string data = vfs->read_file(resolved, true);

    // Append this script's directory to the search path for #includes.
    DSearchPath my_search_path = search_path;
    my_search_path.append_directory(resolved.get_dirname());
    state = parse(data, my_search_path);
  }

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

    } else if (key == "alpha_scale") {
      const ColorScaleAttrib *csa;
      state->get_attrib(csa);

      float scale = atof(value.c_str());

      if (csa == nullptr) {
        state = state->set_attrib(ColorScaleAttrib::make(
          LVecBase4(1.0f, 1.0f, 1.0f, scale)));
      } else {
        const LVecBase4 &curr_scale = csa->get_scale();
        state = state->set_attrib(csa->set_scale(
          LVecBase4(curr_scale[0], curr_scale[1], curr_scale[2], scale)));
      }

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
 * Write the indicated RenderState to a script file on disk.
 */
void RenderStateScript::
write(const RenderState *state, const Filename &filename,
      BamEnums::BamTextureMode mode) {
  Filename write_filename = filename;
  if (write_filename.get_extension().empty()) {
    write_filename = write_filename.get_fullpath() +
      default_render_state_script_extension.get_value();
  }

  if (write_filename.get_extension() == get_binary_extension()) {
    // If they want to write a binary render state script, just serialize the
    // actual RenderState object.

    // We need to clear the existing filename associated with the RenderState
    // so the actual RenderState guts are written, and not just a filename
    // reference.
    state->_filename = "";
    state->_fullpath = "";

    BamFile bam;
    if (!bam.open_write(write_filename)) {
      pgraph_cat.error()
        << "Couldn't open " << write_filename.get_fullpath() << " to write a render "
        << "state script.\n";
      return;
    }

    if (!bam.write_object(state)) {
      pgraph_cat.error()
        << "Couldn't write render state script to " << write_filename.get_fullpath()
        << ".\n";
    }

    // Set the filename we just wrote to on the RenderState so if we write a
    // model file that uses this state, it will reference the filename.
    state->_filename = write_filename;

    bam.close();
    return;
  }

  // We are writing a raw text render state script.

  PT(CKeyValues) script = new CKeyValues;

  const ColorAttrib *ca;
  if (state->get_attrib(ca)) {
    script->set_key_value("color", CKeyValues::to_string(ca->get_color()));
  }

  const ColorScaleAttrib *csa;
  if (state->get_attrib(csa)) {
    script->set_key_value("color_scale", CKeyValues::to_string(csa->get_scale()));
  }

  const DepthWriteAttrib *dwa;
  if (state->get_attrib(dwa)) {
    DepthWriteAttrib::Mode mode = dwa->get_mode();
    script->set_key_value("z_write", mode == DepthWriteAttrib::M_on ? "1" : "0");
  }

  const DepthTestAttrib *dta;
  if (state->get_attrib(dta)) {
    DepthTestAttrib::PandaCompareFunc mode = dta->get_mode();
    script->set_key_value("z_test", mode != DepthTestAttrib::M_none ? "1" : "0");
  }

  const DepthOffsetAttrib *doa;
  if (state->get_attrib(doa)) {
    script->set_key_value("z_offset", CKeyValues::to_string(doa->get_offset()));
  }

  const FogAttrib *fa;
  if (state->get_attrib(fa) && fa->is_off()) {
    script->set_key_value("no_fog", "1");
  }

  const LightAttrib *la;
  if (state->get_attrib(la) && la->has_all_off()) {
    script->set_key_value("no_light", "1");
  }

  const TransparencyAttrib *ta;
  if (state->get_attrib(ta)) {
    switch (ta->get_mode()) {
    case TransparencyAttrib::M_none:
      script->set_key_value("transparency", "off");
      break;
    case TransparencyAttrib::M_alpha:
    default:
      script->set_key_value("transparency", "alpha");
      break;
    case TransparencyAttrib::M_premultiplied_alpha:
      script->set_key_value("transparency", "premultiplied_alpha");
      break;
    case TransparencyAttrib::M_multisample:
      script->set_key_value("transparency", "multisample");
      break;
    case TransparencyAttrib::M_multisample_mask:
      script->set_key_value("transparency", "multisample_mask");
      break;
    case TransparencyAttrib::M_binary:
      script->set_key_value("transparency", "binary");
      break;
    case TransparencyAttrib::M_dual:
      script->set_key_value("transparency", "dual");
      break;
    }
  }

  const ColorWriteAttrib *cwa;
  if (state->get_attrib(cwa)) {
    unsigned int channels = cwa->get_channels();
    if (channels == ColorWriteAttrib::C_off) {
      script->set_key_value("color_write", "off");
    } else if (channels == ColorWriteAttrib::C_all) {
      script->set_key_value("color_write", "all");
    } else {
      std::string channels_str;
      if (channels | ColorWriteAttrib::C_red) {
        channels_str += 'r';
      }
      if (channels | ColorWriteAttrib::C_green) {
        channels_str += 'g';
      }
      if (channels | ColorWriteAttrib::C_blue) {
        channels_str += 'b';
      }
      if (channels | ColorWriteAttrib::C_alpha) {
        channels_str += 'a';
      }
      script->set_key_value("color_write", channels_str);
    }
  }

  const CullFaceAttrib *cfa;
  if (state->get_attrib(cfa)) {
    CullFaceAttrib::Mode mode = cfa->get_effective_mode();
    switch (mode) {
    case CullFaceAttrib::M_cull_none:
      script->set_key_value("cull", "none");
      break;
    case CullFaceAttrib::M_cull_clockwise:
    default:
      script->set_key_value("cull", "cw");
      break;
    case CullFaceAttrib::M_cull_counter_clockwise:
      script->set_key_value("cull", "ccw");
      break;
    }
  }

  const TextureAttrib *tex_attr;
  if (state->get_attrib(tex_attr)) {
    const TexMatrixAttrib *tma;
    state->get_attrib_def(tma);
    for (int i = 0; i < tex_attr->get_num_on_stages(); i++) {
      TextureStage *stage = tex_attr->get_on_stage(i);
      PT(CKeyValues) tex_block = new CKeyValues("texture", script);
      tex_block->set_key_value("stage", stage->get_name());
      Texture *tex = tex_attr->get_on_texture(stage);
      if (tex) {
        if (!tex->get_filename().empty()) {
          tex_block->set_key_value("filename", tex->get_filename().get_fullpath());
          if (!tex->get_alpha_filename().empty()) {
            tex_block->set_key_value("alpha_filename", tex->get_alpha_filename().get_fullpath());
          }
        } else {
          tex_block->set_key_value("name", tex->get_name());
        }
      }

      // If the stage has a specific texcoord name assigned, write that out.
      std::string texcoord_name = stage->get_texcoord_name()->get_name();
      if (!texcoord_name.empty()) {
        tex_block->set_key_value("texcoord", texcoord_name);
      }

      // Write out texture transform if we have it.
      CPT(TransformState) ts = tma->get_transform(stage);
      if (!ts->is_identity()) {
        const LPoint3 &pos = ts->get_pos();
        const LVector3 &hpr = ts->get_hpr();
        const LVector3 &scale = ts->get_scale();

        if (pos != LPoint3(0)) {
          tex_block->set_key_value("pos", CKeyValues::to_string(pos));
        }

        if (hpr != LVector3(0)) {
          tex_block->set_key_value("hpr", CKeyValues::to_string(hpr));
        }

        if (scale != LVector3(1)) {
          tex_block->set_key_value("scale", CKeyValues::to_string(scale));
        }
      }
    }
  }

  const CullBinAttrib *cba;
  if (state->get_attrib(cba)) {
    PT(CKeyValues) cba_block = new CKeyValues("bin", script);
    cba_block->set_key_value("name", cba->get_bin_name());
    cba_block->set_key_value("sort", CKeyValues::to_string(cba->get_draw_order()));
  }

  const AlphaTestAttrib *ata;
  if (state->get_attrib(ata)) {
    PT(CKeyValues) ata_block = new CKeyValues("alpha_test", script);
    ata_block->set_key_value("reference", CKeyValues::to_string(ata->get_reference_alpha()));
    switch (ata->get_mode()) {
    case AlphaTestAttrib::M_never:
      ata_block->set_key_value("compare", "never");
      break;
    case AlphaTestAttrib::M_less:
      ata_block->set_key_value("compare", "less");
      break;
    case AlphaTestAttrib::M_equal:
      ata_block->set_key_value("compare", "equal");
      break;
    case AlphaTestAttrib::M_less_equal:
    default:
      ata_block->set_key_value("compare", "less_equal");
      break;
    case AlphaTestAttrib::M_greater:
      ata_block->set_key_value("compare", "greater");
      break;
    case AlphaTestAttrib::M_greater_equal:
      ata_block->set_key_value("compare", "greater_equal");
      break;
    case AlphaTestAttrib::M_always:
      ata_block->set_key_value("compare", "always");
    }
  }

  const ShaderParamAttrib *spa;
  if (state->get_attrib(spa)) {
    PT(CKeyValues) spa_block = new CKeyValues("shader", script);
    spa_block->set_key_value("name", spa->get_shader_name());
    for (int i = 0; spa->get_num_params(); i++) {
      spa_block->set_key_value(spa->get_param_key(i), spa->get_param_value(i));
    }
  }

  const RenderModeAttrib *rma;
  if (state->get_attrib(rma)) {
    PT(CKeyValues) rma_block = new CKeyValues("render_mode", script);
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

    rma_block->set_key_value("perspective", CKeyValues::to_string(rma->get_perspective()));
    rma_block->set_key_value("wireframe_color", CKeyValues::to_string(rma->get_wireframe_color()));
    rma_block->set_key_value("thickness", CKeyValues::to_string(rma->get_thickness()));
  }

  pgraph_cat.info()
    << "Writing render state script " << write_filename.get_fullpath() << "\n";
  script->write(write_filename, 2);
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
  Filename filename;
  Filename alpha_filename;
  std::string stage_name;
  std::string texcoord_name;
  std::string tex_name;
  LPoint3 pos(0, 0, 0);
  LVector3 hpr(0, 0, 0);
  LVector3 scale(1, 1, 1);
  bool got_transform = false;
  bool cubemap = false;

  for (size_t i = 0; i < block->get_num_keys(); i++) {
    const std::string &key = block->get_key(i);
    const std::string &value = block->get_value(i);

    if (key == "stage") {
      stage_name = value;

    } else if (key == "texcoord") {
      texcoord_name = value;

    } else if (key == "filename") {
      filename = value;

    } else if (key == "alpha_filename") {
      alpha_filename = value;

    } else if (key == "cubemap") {
      cubemap = parse_bool_string(value);

    } else if (key == "name") {
      tex_name = value;

    } else if (key == "pos") {
      pos = CKeyValues::to_3f(value);
      got_transform = true;

    } else if (key == "hpr") {
      hpr = CKeyValues::to_3f(value);
      got_transform = true;

    } else if (key == "scale") {
      scale = CKeyValues::to_3f(value);
      got_transform = true;
    }
  }

  PT(TextureStage) stage;
  if (stage_name.empty()) {
    stage = TextureStage::get_default();
  } else {
    stage = new TextureStage(stage_name);
  }

  if (!texcoord_name.empty()) {
    // They asked for a specific texcoord name to assign to the texture.
    stage->set_texcoord_name(texcoord_name);
  }

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

  // Create a new RenderState that contains just our texture-related
  // attributes.  We will compose the running state with this state, combining
  // any existing TextureAttribs or TexMatrixAttribs.
  CPT(RenderState) tex_state = RenderState::make_empty();
  CPT(RenderAttrib) texattr = TextureAttrib::make();
  texattr = DCAST(TextureAttrib, texattr)->add_on_stage(stage, tex);
  tex_state = tex_state->set_attrib(texattr);

  if (got_transform) {
    CPT(TransformState) ts = TransformState::make_pos_hpr_scale(pos, hpr, scale);
    CPT(RenderAttrib) tma = TexMatrixAttrib::make(stage, ts);
    tex_state = tex_state->set_attrib(tma);
  }

  // Compose the running state with our texture state.
  state = state->compose(tex_state);
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
