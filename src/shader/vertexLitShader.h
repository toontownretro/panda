/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexLitShader.h
 * @author lachbr
 * @date 2020-10-30
 */

#ifndef VERTEXLITSHADER_H
#define VERTEXLITSHADER_H

#include "shaderBase.h"
#include "standardMaterial.h"

/**
 * Shader that renders the StandardMaterial type.
 */
class EXPCL_PANDA_SHADER VertexLitShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec) override;
protected:
  INLINE VertexLitShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "VertexLitShader",
                  ShaderBase::get_class_type());

    StandardMaterial::init_type();
    register_shader(new VertexLitShader, StandardMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vertexLitShader.I"

#endif // VERTEXLITGENERICSHADER_H
