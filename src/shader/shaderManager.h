/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.h
 * @author lachbr
 * @date 2020-10-12
 */

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "config_shader.h"
#include "pmap.h"
#include "geomVertexAnimationSpec.h"
#include "renderAttrib.h"

class ShaderBase;
class RenderState;

/**
 *
 */
class EXPCL_PANDA_SHADER ShaderManager {
PUBLISHED:
  static ShaderManager *get_global_ptr();

public:
  void load_shader_libraries();

  void register_shader(ShaderBase *shader);

  CPT(RenderAttrib) generate_shader(const RenderState *state,
                                    const GeomVertexAnimationSpec &animation_spec);

  INLINE ShaderBase *get_shader(const std::string &name) const;

private:
  typedef pmap<std::string, ShaderBase *> ShaderRegistry;
  ShaderRegistry _shaders;

  static ShaderManager *_global_ptr;
};

#include "shaderManager.I"

#endif // SHADERMANAGER_H
