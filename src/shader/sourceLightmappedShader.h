/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceLightmappedShader.h
 * @author brian
 * @date 2022-03-17
 */

#ifndef SOURCELIGHTMAPPEDSHADER_H
#define SOURCELIGHTMAPPEDSHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "sourceLightmappedMaterial.h"

/**
 * Shader that renders the SourceLightmappedMaterial type.
 */
class EXPCL_PANDA_SHADER SourceLightmappedShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE SourceLightmappedShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "SourceLightmappedShader",
                  ShaderBase::get_class_type());

    SourceLightmappedMaterial::init_type();
    register_shader(new SourceLightmappedShader, SourceLightmappedMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sourceLightmappedShader.I"

#endif // SOURCELIGHTMAPPEDSHADER_H
