/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eyeRefractShader.h
 * @author lachbr
 * @date 2021-03-24
 */

#ifndef EYEREFRACTSHADER_H
#define EYEREFRACTSHADER_H

#include "shaderBase.h"
#include "eyeRefractMaterial.h"

/**
 * Eye shader.
 */
class EXPCL_PANDA_SHADER EyeRefractShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *params,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;

protected:
  INLINE EyeRefractShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "EyeRefractShader",
                  ShaderBase::get_class_type());

    EyeRefractMaterial::init_type();
    register_shader(new EyeRefractShader, EyeRefractMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eyeRefractShader.I"

#endif // EYEREFRACTSHADER_H
