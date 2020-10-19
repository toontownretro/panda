/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file defaultShader.h
 * @author lachbr
 * @date 2020-10-18
 */

#ifndef DEFAULTSHADER_H
#define DEFAULTSHADER_H

#include "config_shader.h"
#include "shaderBase.h"

/**
 * Default shader.  Does nothing but sample a single texture and apply
 * coloring.
 */
class EXPCL_PANDA_SHADER DefaultShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               const ShaderParamAttrib *params,
                               const GeomVertexAnimationSpec &anim_spec) override;
protected:
  INLINE DefaultShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "DefaultShader",
                  ShaderBase::get_class_type());
    register_shader(new DefaultShader);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "defaultShader.I"

#endif // DEFAULTSHADER_H
