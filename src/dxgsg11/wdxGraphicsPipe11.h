/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsPipe11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef WDXGRAPHICSPIPE11_H
#define WDXGRAPHICSPIPE11_H

#include "pandabase.h"
#include "winGraphicsPipe.h"
#include "graphicsDevice.h"

class IDXGIFactory1;

/**
 * This graphics pipe represents the interface for creating DirectX11 graphics
 * contexts and windows.
 */
class EXPCL_PANDA_DXGSG11 wdxGraphicsPipe11 : public WinGraphicsPipe {
public:
  wdxGraphicsPipe11();
  virtual ~wdxGraphicsPipe11();

  static PT(GraphicsPipe) pipe_constructor();

  virtual std::string get_interface_name() const override;

  PT(GraphicsDevice) make_dx_device(GraphicsEngine *engine);

  IDXGIFactory1 *get_dxgi_factory();

protected:
  virtual PT(GraphicsOutput) make_output(const std::string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify) override;

private:
  IDXGIFactory1 *_dxgi_factory;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsPipe::init_type();
    register_type(_type_handle, "wdxGraphicsPipe11",
                  WinGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wdxGraphicsPipe11.I"

#endif // WDXGRAPHICSPIPE11_H
