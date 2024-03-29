/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file noMatShader.h
 * @author brian
 * @date 2021-03-22
 */

#ifndef NOMATSHADER_H
#define NOMATSHADER_H

#include "pandabase.h"
#include "config_shader.h"
#include "shaderBase.h"

/**
 * This is the shader that gets used for render states that do not contain a
 * material.  Used for single-textured (through TextureAttrib) unlit geometry,
 * such as UI elements and sprites.
 */
class EXPCL_PANDA_SHADER NoMatShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE NoMatShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "NoMatShader",
                  ShaderBase::get_class_type());
    if (!config_get_use_vertex_lit_for_no_material()) {
      // Not using VertexLit for no material, so register ourselves.
      register_shader(new NoMatShader, TypeHandle::none());
    }
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "noMatShader.I"

#endif // NOMATSHADER_H
