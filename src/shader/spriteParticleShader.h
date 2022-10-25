/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteParticleShader.h
 * @author brian
 * @date 2021-09-01
 */

#ifndef SPRITEPARTICLESHADER_H
#define SPRITEPARTICLESHADER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "spriteParticleMaterial.h"

/**
 * Shader that renders point sprite particles.
 */
class EXPCL_PANDA_SHADER SpriteParticleShader : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE SpriteParticleShader();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "SpriteParticleShader",
                  ShaderBase::get_class_type());
    SpriteParticleMaterial::init_type();
    register_shader(new SpriteParticleShader, SpriteParticleMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "spriteParticleShader.I"

#endif // SPRITEPARTICLESHADER_H
