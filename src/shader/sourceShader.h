/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceShader.h
 * @author brian
 * @date 2021-10-25
 */

#ifndef SOURCESHADER_H
#define SOURCESHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "sourceMaterial.h"

/**
 * Shader that renders the SourceMaterial type.
 */
class EXPCL_PANDA_SHADER SourceShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE SourceShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "SourceShader",
                  ShaderBase::get_class_type());

    SourceMaterial::init_type();
    register_shader(new SourceShader, SourceMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sourceShader.I"

#endif // SOURCESHADER_H
