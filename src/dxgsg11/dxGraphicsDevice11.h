/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsDevice11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXGRAPHICSDEVICE11_H
#define DXGRAPHICSDEVICE11_H

#include "pandabase.h"
#include "graphicsDevice.h"
#include "dxGraphicsStateGuardian11.h"
#include "pointerTo.h"

#include <d3d11.h>

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXGraphicsDevice11 : public GraphicsDevice {
public:
  DXGraphicsDevice11(GraphicsPipe *pipe, GraphicsEngine *engine);

  bool initialize();

  INLINE IDXGIAdapter1 *get_adapter() const;
  INLINE ID3D11Device *get_device() const;
  INLINE ID3D11DeviceContext *get_context() const;

  INLINE D3D_FEATURE_LEVEL get_feature_level() const;

  DXGraphicsStateGuardian11 *get_gsg();

private:
  IDXGIAdapter1 *_adapter;
  ID3D11Device *_device;
  ID3D11DeviceContext *_context;

  GraphicsEngine *_engine;

  PT(DXGraphicsStateGuardian11) _dxgsg;

  bool _device_initialized;

  D3D_FEATURE_LEVEL _feature_level;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsDevice::init_type();
    register_type(_type_handle, "DXGraphicsDevice11",
                  GraphicsDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGraphicsDevice11.I"

#endif // DXGRAPHICSDEVICE11_H
