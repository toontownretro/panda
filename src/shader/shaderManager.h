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
#include "shaderManagerBase.h"
#include "pmap.h"
#include "geomVertexAnimationSpec.h"
#include "renderAttrib.h"
#include "shaderEnums.h"

class ShaderBase;
class RenderState;
class GraphicsStateGuardianBase;

/**
 * This class is responsible for the registry of available shaders and calling
 * upon a shader to generate a shader for a given RenderState.
 */
class EXPCL_PANDA_SHADER ShaderManager : public ShaderEnums, public ShaderManagerBase {
private:
  INLINE ShaderManager();

PUBLISHED:
  INLINE void set_shader_quality(ShaderQuality quality);
  INLINE ShaderQuality get_shader_quality() const;
  MAKE_PROPERTY(shader_quality, get_shader_quality, set_shader_quality);

  INLINE static ShaderManager *get_global_ptr();

public:
  void load_shader_libraries();

  void register_shader(ShaderBase *shader);

  virtual CPT(RenderAttrib) generate_shader(GraphicsStateGuardianBase *gsg,
                                            const RenderState *state,
                                            const GeomVertexAnimationSpec &anim_spec) override;

  INLINE ShaderBase *get_shader(const std::string &name) const;

private:
  typedef phash_map<std::string, ShaderBase *, string_hash> ShaderRegistry;
  ShaderRegistry _shaders;

  ShaderQuality _quality;

  static ShaderManager *_global_ptr;
};

#include "shaderManager.I"

#endif // SHADERMANAGER_H
