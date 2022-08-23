/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file basicShader.h
 * @author brian
 * @date 2022-03-16
 */

#ifndef BASICSHADER_H
#define BASICSHADER_H

#include "pandabase.h"
#include "shaderBase.h"

/**
 * Very basic shader suitable for rendering most of Toontown.
 * Provides single texturing w/ texture matrix, hardware skinning,
 * alpha testing, fogging, clipping, and vertex colors/color scale.
 *
 * It is also the default shader used when a RenderState doesn't have
 * Material.
 *
 * First shader to use precompiled combo system and specialization
 * constants.
 */
class EXPCL_PANDA_SHADER BasicShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE BasicShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "BasicShader",
                  ShaderBase::get_class_type());
    // Register it with material type "none" to fallback to this shader
    // when there's no material.
    register_shader(new BasicShader, TypeHandle::none());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "basicShader.I"

#endif // BASICSHADER_H
