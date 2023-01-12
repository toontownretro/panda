/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthShader.h
 * @author brian
 * @date 2020-12-16
 */

#ifndef DEPTHSHADER_H
#define DEPTHSHADER_H

#include "pandabase.h"
#include "shaderBase.h"

/**
 * Generates a shader for rendering to a single shadow depth map.
 */
class DepthShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *params,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;

protected:
  INLINE DepthShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "DepthShader",
                  ShaderBase::get_class_type());
    register_shader(new DepthShader);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "depthShader.I"

#endif // DEPTHSHADER_H
