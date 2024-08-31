/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pbrShader.h
 * @author brian
 * @date 2024-08-30
 */

#ifndef PBRSHADER_H
#define PBRSHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "pbrMaterial.h"

/**
 * Shader that renders the PBRMaterial type.
 */
class EXPCL_PANDA_SHADER PBRShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE PBRShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "PBRShader",
                  ShaderBase::get_class_type());

    PBRMaterial::init_type();
    register_shader(new PBRShader, PBRMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pbrShader.I"

#endif // PBRSHADER_H
