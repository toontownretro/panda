/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightmappedShader.h
 * @author lachbr
 * @date 2021-01-02
 */

#if 0

#ifndef LIGHTMAPPEDSHADER_H
#define LIGHTMAPPEDSHADER_H

#include "shaderBase.h"

/**
 * Shader that renders lightmapped brush geometry.
 */
class EXPCL_PANDA_SHADER LightmappedShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *params,
                               const GeomVertexAnimationSpec &anim_spec) override;
protected:
  INLINE LightmappedShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "LightmappedShader",
                  ShaderBase::get_class_type());
    register_shader(new LightmappedShader);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lightmappedShader.I"

#endif // LIGHTMAPPEDSHADER_H

#endif
