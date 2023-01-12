/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderManager.h
 * @author brian
 * @date 2020-10-12
 */

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "config_shader.h"
#include "shaderManagerBase.h"
#include "pmap.h"
#include "geomVertexAnimationSpec.h"
#include "graphicsStateGuardianBase.h"
#include "renderAttrib.h"
#include "shaderEnums.h"
#include "internalName.h"
#include "texture.h"

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

  void reload_shaders();

  INLINE void set_default_cube_map(Texture *texture);
  Texture *get_default_cube_map();

  INLINE static ShaderManager *get_global_ptr();

public:
  void load_shader_libraries();

  void register_shader(ShaderBase *shader);
  void register_shader(ShaderBase *shader, TypeHandle material_type);

  virtual CPT(RenderAttrib) generate_shader(GraphicsStateGuardianBase *gsg,
                                            const RenderState *state,
                                            const GeomVertexAnimationSpec &anim_spec) override;

  INLINE ShaderBase *get_shader(CPT_InternalName name) const;

private:
  typedef pflat_hash_map<CPT(InternalName), ShaderBase *> ShaderRegistry;
  // This maps material types to the shader that can render it.
  typedef pflat_hash_map<TypeHandle, ShaderBase *> MaterialShaders;
  ShaderRegistry _shaders;
  MaterialShaders _material_shaders;

  ShaderQuality _quality;

  PT(Texture) _default_cubemap;

  static ShaderManager *_global_ptr;
};

#include "shaderManager.I"

#endif // SHADERMANAGER_H
