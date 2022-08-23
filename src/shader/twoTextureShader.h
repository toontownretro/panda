/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file twoTextureShader.h
 * @author brian
 * @date 2022-03-22
 */

#ifndef TWOTEXTURESHADER_H
#define TWOTEXTURESHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "twoTextureMaterial.h"

/**
 * Shader that renders the TwoTextureMaterial type.
 */
class EXPCL_PANDA_SHADER TwoTextureShader final : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE TwoTextureShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "TwoTextureShader",
                  ShaderBase::get_class_type());

    TwoTextureMaterial::init_type();
    register_shader(new TwoTextureShader, TwoTextureMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "twoTextureShader.I"

#endif // TWOTEXTURESHADER_H
