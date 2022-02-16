/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceSkyShader.h
 * @author brian
 * @date 2022-02-15
 */

#ifndef SOURCESKYSHADER_H
#define SOURCESKYSHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "sourceSkyMaterial.h"

/**
 * Shader that renders the SourceSkyMaterial type.
 */
class EXPCL_PANDA_SHADER SourceSkyShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec) override;
protected:
  INLINE SourceSkyShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "SourceSkyShader",
                  ShaderBase::get_class_type());

    SourceSkyMaterial::init_type();
    register_shader(new SourceSkyShader, SourceSkyMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sourceSkyShader.I"

#endif // SOURCESKYSHADER_H
