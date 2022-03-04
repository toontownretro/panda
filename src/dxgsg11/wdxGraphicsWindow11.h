/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsWindow11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef WDXGRAPHICSWINDOW11_H
#define WDXGRAPHICSWINDOW11_H

#include "pandabase.h"
#include "winGraphicsWindow.h"

class IDXGISwapChain;
class DXGraphicsDevice11;
class ID3D11Texture2D;
class ID3D11RenderTargetView;
class ID3D11DepthStencilView;

/**
 *
 */
class EXPCL_PANDA_DXGSG11 wdxGraphicsWindow11 : public WinGraphicsWindow {
public:
  wdxGraphicsWindow11(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);

  virtual bool begin_frame(FrameMode mode, Thread *current_thread) override;
  virtual void end_frame(FrameMode mode, Thread *current_thread) override;

  virtual bool open_window() override;

  bool create_swap_chain();

  virtual void end_flip() override;

private:
  IDXGISwapChain *_swap_chain;
  DXGraphicsDevice11 *_dx_device;

  ID3D11Texture2D *_back_buffer;
  ID3D11RenderTargetView *_back_buffer_view;

  ID3D11Texture2D *_depth_buffer;
  ID3D11DepthStencilView *_depth_stencil_view;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsWindow::init_type();
    register_type(_type_handle, "wdxGraphicsWindow11",
                  WinGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wdxGraphicsWindow11.I"

#endif // WDXGRAPHICSWINDOW11_H
