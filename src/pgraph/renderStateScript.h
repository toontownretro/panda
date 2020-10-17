/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderStateScript.h
 * @author lachbr
 * @date 2020-10-13
 */

#ifndef RENDERSTATESCRIPT_H
#define RENDERSTATESCRIPT_H

#include "config_pgraph.h"
#include "filename.h"
#include "pmap.h"
#include "renderState.h"
#include "lightMutex.h"

class CKeyValues;

/**
 * Text file representation of a RenderState.
 */
class EXPCL_PANDA_PGRAPH RenderStateScript {
PUBLISHED:
  static CPT(RenderState) load(const Filename &filename);
  static CPT(RenderState) parse(const std::string &data);

private:
  RenderStateScript() = delete;

  static bool parse_bool_string(const std::string &value);
  static bool is_true_string(const std::string &value);
  static void parse_texture_block(CKeyValues *block, CPT(RenderState) &state);
  static void parse_bin_block(CKeyValues *block, CPT(RenderState) &state);
  static void parse_alpha_test_block(CKeyValues *block, CPT(RenderState) &state);
  static void parse_shader_block(CKeyValues *block, CPT(RenderState) &state);
  static void parse_render_mode_block(CKeyValues *block, CPT(RenderState) &state);
  static void parse_color_blend_block(CKeyValues *block, CPT(RenderState) &state);
  static void parse_color_write(const std::string &value, CPT(RenderState) &state);

  typedef pmap<Filename, CPT(RenderState)> ScriptCache;
  static ScriptCache _cache;

  static LightMutex _mutex;
};

#include "renderStateScript.I"

#endif // RENDERSTATESCRIPT_H
