/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file csmDepthShader.h
 * @author lachbr
 * @date 2020-10-30
 */

#ifndef CSMDEPTHSHADER_H
#define CSMDEPTHSHADER_H

#include "shaderBase.h"

/**
 * Generates a shader that renders geometry to cascaded shadow depth maps.
 */
class EXPCL_PANDA_SHADER CSMDepthShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec) override;
protected:
  INLINE CSMDepthShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "CSMDepthShader",
                  ShaderBase::get_class_type());
    register_shader(new CSMDepthShader);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "csmDepthShader.I"

#endif // CSMDEPTHSHADER_H
