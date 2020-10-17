/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderBase.h
 * @author lachbr
 * @date 2020-10-16
 */

#ifndef SHADERBASE_H
#define SHADERBASE_H

#include "config_shader.h"
#include "typedObject.h"
#include "namable.h"

/**
 *
 */
class EXPCL_PANDA_SHADER ShaderBase : public TypedObject, public Namable {
public:
  //virtual
protected:
  INLINE ShaderBase(const std::string &name);

  static void register_shader(ShaderBase *shader);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "ShaderBase",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "shaderBase.I"

#endif // SHADERBASE_H
