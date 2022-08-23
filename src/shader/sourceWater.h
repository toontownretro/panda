/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceWater.h
 * @author brian
 * @date 2022-07-10
 */

#ifndef SOURCEWATER_H
#define SOURCEWATER_H

#include "pandabase.h"
#include "shaderBase.h"
#include "sourceWaterMaterial.h"

class EXPCL_PANDA_SHADER SourceWater : public ShaderBase {
public:
  virtual void generate_shader(GraphicsStateGuardianBase *gsg,
                               const RenderState *state,
                               Material *material,
                               const GeomVertexAnimationSpec &anim_spec,
                               ShaderSetup &setup) override;
protected:
  INLINE SourceWater();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderBase::init_type();
    register_type(_type_handle, "SourceWater",
                  ShaderBase::get_class_type());

    SourceWaterMaterial::init_type();
    register_shader(new SourceWater, SourceWaterMaterial::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sourceWater.I"

#endif // SOURCEWATER_H
