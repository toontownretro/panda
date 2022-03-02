/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsDevice11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxGraphicsDevice11.h"
#include "config_dxgsg11.h"
#include "wdxGraphicsPipe11.h"
#include "graphicsEngine.h"

TypeHandle DXGraphicsDevice11::_type_handle;

/**
 *
 */
DXGraphicsDevice11::
DXGraphicsDevice11(GraphicsPipe *pipe, GraphicsEngine *engine) :
  GraphicsDevice(pipe),
  _engine(engine),
  _adapter(nullptr),
  _context(nullptr),
  _device(nullptr),
  _dxgsg(nullptr),
  _device_initialized(false)
{
}

/**
 *
 */
bool DXGraphicsDevice11::
initialize() {
  if (_device_initialized) {
    return true;
  }

  _device_initialized = true;

  IDXGIFactory1 *factory = DCAST(wdxGraphicsPipe11, _pipe)->get_dxgi_factory();
  nassertr(factory != nullptr, false);

  pvector<IDXGIAdapter1 *> adapters;
  IDXGIAdapter1 *adapter = nullptr;
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    adapters.push_back(adapter);
  }

  if (adapters.empty()) {
    dxgsg11_cat.error()
      << "No available graphics adapters!\n";
    return false;
  }

  _adapter = adapters[0];

  nassertr(_adapter != nullptr, false);

  dxgsg11_cat.info()
    << "Using first available adapter\n";
  DXGI_ADAPTER_DESC1 adesc;
  _adapter->GetDesc1(&adesc);

  dxgsg11_cat.info()
    << "Adapter info:\n"
    << "\tDescription: " << std::wstring(adesc.Description) << "\n"
    << "\tVendorId: " << adesc.VendorId << "\n"
    << "\tDeviceId: " << adesc.DeviceId << "\n"
    << "\tSubSysId: " << adesc.SubSysId << "\n"
    << "\tRevision: " << adesc.Revision << "\n"
    << "\tDedicatedVideoMemory: " << adesc.DedicatedVideoMemory / 1000000 << " MB\n"
    << "\tDedicatedSystemMemory: " << adesc.DedicatedSystemMemory / 1000000 << " MB\n"
    << "\tSharedSystemMemory: " << adesc.SharedSystemMemory / 1000000 << " MB\n"
    << "\tAdapterLuid: " << adesc.AdapterLuid.HighPart << " " << adesc.AdapterLuid.LowPart << "\n"
    << "\tFlags: " << adesc.Flags << "\n";

  if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
    dxgsg11_cat.info(false)
      << "\tAdapter type: Software\n";
  } else {
    dxgsg11_cat.info(false)
      << "\tAdapter type: Hardware\n";
  }

  UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
  if (dxgsg11_cat.is_debug()) {
    flags |= D3D11_CREATE_DEVICE_DEBUG;
  }
  D3D_FEATURE_LEVEL device_feature_level = D3D_FEATURE_LEVEL_9_1;
  D3D_FEATURE_LEVEL possible_feature_levels[9] = {
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1,
  };
  HRESULT result = D3D11CreateDevice(_adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags,
                                     possible_feature_levels, 9, D3D11_SDK_VERSION, &_device,
                                     &device_feature_level, &_context);
  if (FAILED(result)) {
    dxgsg11_cat.error()
      << "Failed to create D3D11 device! (" << result << ")\n";
    return false;
  }

  nassertr(_device != nullptr, false);

  dxgsg11_cat.info()
    << "Device D3D feature level: ";
  switch (device_feature_level) {
  case D3D_FEATURE_LEVEL_9_1:
    dxgsg11_cat.info(false)
      << "9.1\n";
    break;
  case D3D_FEATURE_LEVEL_9_2:
    dxgsg11_cat.info(false)
      << "9.2\n";
    break;
  case D3D_FEATURE_LEVEL_9_3:
    dxgsg11_cat.info(false)
      << "9.3\n";
    break;
  case D3D_FEATURE_LEVEL_10_0:
    dxgsg11_cat.info(false)
      << "10.0\n";
    break;
  case D3D_FEATURE_LEVEL_10_1:
    dxgsg11_cat.info(false)
      << "10.1\n";
    break;
  case D3D_FEATURE_LEVEL_11_0:
    dxgsg11_cat.info(false)
      << "11.0\n";
    break;
  case D3D_FEATURE_LEVEL_11_1:
    dxgsg11_cat.info(false)
      << "11.1\n";
    break;
  case D3D_FEATURE_LEVEL_12_0:
    dxgsg11_cat.info(false)
      << "12.0\n";
    break;
  case D3D_FEATURE_LEVEL_12_1:
    dxgsg11_cat.info(false)
      << "12.1\n";
    break;
  case D3D_FEATURE_LEVEL_1_0_CORE:
    dxgsg11_cat.info(false)
      << "1.0 core\n";
    break;
  default:
    dxgsg11_cat.info(false)
      << "invalid\n";
    break;
  }

  return true;
}

/**
 * Returns the GSG that should be used to render using this device.
 * Creates it if it has not already been created.
 */
DXGraphicsStateGuardian11 *DXGraphicsDevice11::
get_gsg() {
  nassertr(_device_initialized && _device != nullptr && _context != nullptr, nullptr);

  if (_dxgsg == nullptr) {
    _dxgsg = new DXGraphicsStateGuardian11(_engine, _pipe, _device, _context);
  }

  return _dxgsg;
}
