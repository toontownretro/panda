/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsWindow11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "wdxGraphicsWindow11.h"
#include "dxGraphicsStateGuardian11.h"
#include "dxGraphicsDevice11.h"
#include "wdxGraphicsPipe11.h"
#include "config_dxgsg11.h"
#include "config_display.h"

#include <d3d11.h>

TypeHandle wdxGraphicsWindow11::_type_handle;

/**
 *
 */
wdxGraphicsWindow11::
wdxGraphicsWindow11(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  WinGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _swap_chain(nullptr),
  _dx_device(nullptr),
  _back_buffer(nullptr),
  _back_buffer_view(nullptr),
  _depth_buffer(nullptr),
  _depth_stencil_view(nullptr)
{
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool wdxGraphicsWindow11::
open_window() {
  wdxGraphicsPipe11 *pipe;
  DCAST_INTO_R(pipe, _pipe, false);

  if (pipe->get_device() == nullptr) {
    // This is the first window being opened.  Initialize our graphics device.
    pipe->make_dx_device(_engine);
  }

  DCAST_INTO_R(_dx_device, pipe->get_device(), false);
  // We should have a valid graphics device at this point.
  nassertr(_dx_device != nullptr, false);

  if (_gsg == nullptr) {
    // Grab the GSG associated with the graphics device.
    _gsg = _dx_device->get_gsg();

  } else {
    // A GSG was already assigned to the window.  It better be the one
    // associated with the graphics device.
    if (_gsg != _dx_device->get_gsg()) {
      dxgsg11_cat.warning()
        << "The GSG assigned to this wdxGraphicsWindow11 is different from the GSG "
        << "assigned to the graphics device!  There should be one GSG per "
        << "graphics device.  The window will be forced to use the GSG assigned "
        << "to the graphics device.\n";

      _gsg = _dx_device->get_gsg();
    }
  }

  if (!WinGraphicsWindow::open_window()) {
    return false;
  }

  // Now that we've got the window, we can create a swap chain for it.
  if (!create_swap_chain()) {
    return false;
  }

  return true;
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool wdxGraphicsWindow11::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);

  if (_gsg == nullptr) {
    return false;
  }

  _gsg->reset_if_new();
  _gsg->set_current_properties(&get_fb_properties());
  bool ret = _gsg->begin_frame(current_thread);
  DXGraphicsStateGuardian11 *dxgsg;
  DCAST_INTO_R(dxgsg, _gsg, false);
  dxgsg->set_render_targets(&_back_buffer_view, 1, _depth_stencil_view);
  return ret;
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void wdxGraphicsWindow11::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  if (_gsg == nullptr) {
    return;
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
  }
}

/**
 *
 */
void wdxGraphicsWindow11::
end_flip() {
  if (_swap_chain != nullptr && _flip_ready) {
    HRESULT result = _swap_chain->Present(0, 0);
    if (FAILED(result)) {
      dxgsg11_cat.error()
        << "Failed to present swap chain: " << result << "\n";
    }
  }

  WinGraphicsWindow::end_flip();
}

/**
 *
 */
bool wdxGraphicsWindow11::
create_swap_chain() {
  if (_swap_chain != nullptr) {
    return true;
  }

  wdxGraphicsPipe11 *pipe;
  DCAST_INTO_R(pipe, _pipe, false);
  nassertr(pipe != nullptr, false);

  nassertr(_dx_device != nullptr, false);

  ID3D11Device *d3d_device = _dx_device->get_device();
  nassertr(d3d_device != nullptr, false);

  IDXGIAdapter1 *adapter = _dx_device->get_adapter();
  nassertr(adapter != nullptr, false);

  IDXGIFactory1 *factory = pipe->get_dxgi_factory();
  nassertr(factory != nullptr, false);

  WindowProperties win_props = get_properties();
  const FrameBufferProperties &fb_props = get_fb_properties();

  DXGI_SWAP_CHAIN_DESC sdesc;
  ZeroMemory(&sdesc, sizeof(sdesc));
  sdesc.BufferCount = fb_props.get_back_buffers();
  sdesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  sdesc.Windowed = !win_props.get_fullscreen();
  sdesc.OutputWindow = _hWnd;
  if (win_props.get_fullscreen()) {
    sdesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  }

  //if (!sync_video) {
  //  sdesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
  //}
  //if (win_props.get_fullscreen()) {
  //  sdesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FULLSCREEN_VIDEO;
  //}
  sdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_SHADER_INPUT;
  sdesc.BufferDesc.Height = win_props.get_y_size();
  sdesc.BufferDesc.Width = win_props.get_x_size();
  sdesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sdesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sdesc.BufferDesc.RefreshRate.Numerator = 1;
  sdesc.BufferDesc.RefreshRate.Denominator = 60;
  sdesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  if (fb_props.get_multisamples() > 0) {
    sdesc.SampleDesc.Count = fb_props.get_multisamples();
    sdesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
  } else {
    sdesc.SampleDesc.Count = 1;
    sdesc.SampleDesc.Quality = 0;
  }

  HRESULT result = factory->CreateSwapChain(d3d_device, &sdesc, &_swap_chain);
  if (FAILED(result)) {
    dxgsg11_cat.error()
      << "Failed to create swap chain for graphics window! (" << result << ")\n";
    return false;
  }

  nassertr(_swap_chain != nullptr, false);

  // Grab the back buffer texture and initialize the render target.
  ID3D11Texture2D *back_buffer = nullptr;
  result = _swap_chain->GetBuffer(0, IID_ID3D11Texture2D, (void **)&back_buffer);
  nassertr(SUCCEEDED(result), false);

  _back_buffer = back_buffer;

  CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc(back_buffer, D3D11_RTV_DIMENSION_TEXTURE2D);
  result = d3d_device->CreateRenderTargetView(back_buffer, &rtv_desc, &_back_buffer_view);
  nassertr(SUCCEEDED(result) && _back_buffer_view != nullptr, false);

  if (fb_props.get_depth_bits() > 0) {
    // Also create the depth buffer.
    DXGI_FORMAT depth_buffer_format;
    if (fb_props.get_stencil_bits() == 0) {
      // Depth only, no stencil.
      switch (fb_props.get_depth_bits()) {
      case 1:
      case 8:
      case 16:
      default:
        depth_buffer_format = DXGI_FORMAT_D16_UNORM;
        break;
      case 24:
        depth_buffer_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
      case 32:
        depth_buffer_format = DXGI_FORMAT_D32_FLOAT;
        break;
      }
    } else {
      // Depth-stencil.
      switch (fb_props.get_depth_bits()) {
      case 1:
      case 8:
      case 16:
      case 24:
      default:
        depth_buffer_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
      case 32:
        depth_buffer_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        break;
      }
    }
    CD3D11_TEXTURE2D_DESC dsdesc(depth_buffer_format, win_props.get_x_size(), win_props.get_y_size(), 1, 0, D3D11_BIND_DEPTH_STENCIL);
    result = d3d_device->CreateTexture2D(&dsdesc, nullptr, &_depth_buffer);
    nassertr(SUCCEEDED(result) && _depth_buffer != nullptr, false);

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc(D3D11_DSV_DIMENSION_TEXTURE2D, depth_buffer_format);
    result = d3d_device->CreateDepthStencilView(_depth_buffer, &dsv_desc, &_depth_stencil_view);
    nassertr(SUCCEEDED(result) && _depth_stencil_view != nullptr, false);
  }



  return true;
}
