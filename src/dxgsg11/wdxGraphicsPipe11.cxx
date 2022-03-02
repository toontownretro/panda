/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsPipe11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "wdxGraphicsPipe11.h"
#include "dxGraphicsStateGuardian11.h"
#include "wdxGraphicsWindow11.h"
#include "wdxGraphicsBuffer11.h"
#include "dxGraphicsDevice11.h"
#include "config_dxgsg11.h"

#include <d3d11.h>

TypeHandle wdxGraphicsPipe11::_type_handle;

/**
 *
 */
wdxGraphicsPipe11::
wdxGraphicsPipe11() :
  _dxgi_factory(nullptr)
{
}

/**
 *
 */
wdxGraphicsPipe11::
~wdxGraphicsPipe11() {
  if (_dxgi_factory != nullptr) {
    _dxgi_factory->Release();
  }
}

/**
 * Returns a pointer to the IDXGIFactory object, which is used for creating
 * swap chains and enumerating available graphics devices and display modes.
 */
IDXGIFactory1 *wdxGraphicsPipe11::
get_dxgi_factory() {
  if (_dxgi_factory == nullptr) {
    HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&_dxgi_factory);
    if (FAILED(result)) {
      dxgsg11_cat.fatal()
        << "Failed to create DXGIFactory!\n";
    }
    nassertr(_dxgi_factory != nullptr, nullptr);
  }

  return _dxgi_factory;
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default wdxGraphicsPipe11.
 */
PT(GraphicsPipe) wdxGraphicsPipe11::
pipe_constructor() {
  return new wdxGraphicsPipe11;
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string wdxGraphicsPipe11::
get_interface_name() const {
  return "DirectX11";
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) wdxGraphicsPipe11::
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return nullptr;
  }

  DXGraphicsStateGuardian11 *wdxgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(wdxgsg, gsg, nullptr);
  }

  // First thing to try: a visible window.

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if ((fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_float() > 0)) {
        return nullptr;
      }
    }
    return new wdxGraphicsWindow11(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }

  // Second thing to try: a wdxGraphicsBuffer11

  if (retry == 1) {
    if ((!support_render_texture)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if (fb_prop.get_indexed_color() ||
          (fb_prop.get_back_buffers() > 0)||
          (fb_prop.get_accum_bits() > 0)||
          (fb_prop.get_multisamples() > 0)) {
        return nullptr;
      }
    }

    // Early success - if we are sure that this buffer WILL meet specs, we can
    // precertify it.  This looks rather overly optimistic -- ie, buggy.
    //if ((wdxgsg != nullptr) && wdxgsg->is_valid() && !wdxgsg->needs_reset() &&
    //    wdxgsg->get_supports_render_texture()) {
    //  precertify = true;
    //}
    return new wdxGraphicsBuffer11(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }

  // Nothing else left to try.
  return nullptr;
}

/**
 *
 */
PT(GraphicsDevice) wdxGraphicsPipe11::
make_dx_device(GraphicsEngine *engine) {
  PT(DXGraphicsDevice11) device = new DXGraphicsDevice11(this, engine);
  if (!device->initialize()) {
    return nullptr;
  }

  _device = device;
  return device;
}
