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

#include <d3d11.h>

class DXGraphicsStateGuardian11;
class Texture;
class PreparedGraphicsObjects;

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXTextureContext11 : public TextureContext {
public:
  DXTextureContext11(PreparedGraphicsObjects *pgo, DXGraphicsStateGuardian11 *gsg, Texture *tex);
  DXTextureContext11(PreparedGraphicsObjects *pgo, DXGraphicsStateGuardian11 *gsg, ID3D11Texture2D *d3d_tex);
  virtual ~DXTextureContext11();

  bool create_texture();
  bool create_1d_texture();
  bool create_2d_texture();
  bool create_3d_texture();

  bool upload_texture(ID3D11DeviceContext *context);

  void create_srv();

  void create_rtv();
  void create_dsv();

  void compute_d3d_format_and_ram_swizzle();

private:
  Texture *_texture;

  ID3D11Device *_device;

  ID3D11Resource *_d3d_tex;
  ID3D11ShaderResourceView *_d3d_srv;
  ID3D11RenderTargetView *_d3d_rtv;
  ID3D11DepthStencilView *_d3d_dsv;

  // The actual D3D format being used for this texture, regardless of what the
  // Texture says.
  DXGI_FORMAT _actual_d3d_format;
  std::string _ram_image_swizzle;
  int _actual_num_components;
  bool _needs_swizzle;

  friend class DXGraphicsStateGuardian11;

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
