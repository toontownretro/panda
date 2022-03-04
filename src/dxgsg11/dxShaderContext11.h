/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxShaderContext11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXSHADERCONTEXT11_H
#define DXSHADERCONTEXT11_H

#include "pandabase.h"
#include "shaderContext.h"
#include "shader.h"
#include "dxGraphicsStateGuardian11.h"

#include <d3d11.h>

class GeomVertexFormat;
class ShaderModule;

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXShaderContext11 : public ShaderContext {
public:
  DXShaderContext11(Shader *shader, DXGraphicsStateGuardian11 *gsg);
  virtual ~DXShaderContext11();

  ID3D11InputLayout *get_input_layout(const GeomVertexFormat *format, BitMask32 &enabled_arrays);

public:
  ID3D11Device *_device;

  ID3D11VertexShader *_vshader;
  ID3D11GeometryShader *_gshader;
  ID3D11PixelShader *_pshader;

  class InputLayout {
  public:
    BitMask32 _enabled_arrays;
    ID3D11InputLayout *_layout;
  };

  // Cache of D3D input layout objects for each vertex format that uses the
  // shader.
  typedef pflat_hash_map<const GeomVertexFormat *, InputLayout, pointer_hash> InputLayoutCache;
  InputLayoutCache _input_layouts;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ShaderContext::init_type();
    register_type(_type_handle, "DXShaderContext11",
                  ShaderContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxShaderContext11.I"

#endif // DXSHADERCONTEXT11_H
