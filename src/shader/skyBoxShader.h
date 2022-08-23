/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file skyBoxShader.h
 * @author brian
 * @date 2021-07-13
 */

#ifndef SKYBOXSHADER_H
#define SKYBOXSHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "skyBoxMaterial.h"

/**
 * Generates a shader for rendering skybox materials.
 */
class SkyBoxShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *params,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;

protected:
  INLINE SkyBoxShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "SkyBoxShader",
                  ShaderBase::get_class_type());
    SkyBoxMaterial::init_type();
    register_shader(new SkyBoxShader, SkyBoxMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "skyBoxShader.I"

#endif // SKYBOXSHADER_H
