/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxTextureContext11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXTEXTURECONTEXT11_H
#define DXTEXTURECONTEXT11_H

#include "pandabase.h"
#include "textureContext.h"

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXTextureContext11 : public TextureContext {
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext11",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxTextureContext11.I"

#endif // DXTEXTURECONTEXT11_H
