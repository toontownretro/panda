/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxBufferBase11.cxx
 * @author brian
 * @date 2022-03-02
 */

#include "dxBufferBase11.h"
#include "config_dxgsg11.h"

/**
 *
 */
DXBufferBase11::
DXBufferBase11(ID3D11Device *device, D3D11_BIND_FLAG bind_flags) :
  _device(device),
  _buffer(nullptr),
  _immutable(false),
  _dynamic(false),
  _bind_flags(bind_flags),
  _buffer_size(0)
{
}

/**
 *
 */
DXBufferBase11::
~DXBufferBase11() {
  if (_buffer != nullptr) {
    _buffer->Release();
  }
}

/**
 * Creates an ID3D11Buffer with the given parameters and optional initial
 * data from client_pointer.  Note that initial data is required if the
 * UsageHint is UH_static.
 */
bool DXBufferBase11::
create_buffer(GeomEnums::UsageHint usage, size_t size,
              const unsigned char *client_pointer) {
  if (_buffer != nullptr) {
    // Release existing buffer.
    _buffer->Release();
    _buffer = nullptr;
  }

  if (size == 0u) {
    return false;
  }

  D3D11_BUFFER_DESC desc;
  desc.MiscFlags = 0;
  desc.CPUAccessFlags = 0;
  desc.StructureByteStride = 0;

  switch (usage) {
  case GeomEnums::UH_static:
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    _immutable = true;
    break;
  case GeomEnums::UH_dynamic:
  case GeomEnums::UH_stream:
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    _dynamic = true;
    break;
  case GeomEnums::UH_unspecified:
  default:
    desc.Usage = D3D11_USAGE_DEFAULT;
    break;
  }
  desc.ByteWidth = size;
  desc.BindFlags = _bind_flags;

  _buffer_size = size;

  if (_immutable && client_pointer == nullptr) {
    // Immutable buffers must have initial data.
    return false;
  }

  HRESULT hr;
  if (client_pointer != nullptr) {
    // Fill in initial data.  If the buffer is immutable, this is the final
    // data.
    D3D11_SUBRESOURCE_DATA initial_data;
    initial_data.pSysMem = client_pointer;
    initial_data.SysMemPitch = 0;
    initial_data.SysMemSlicePitch = 0;
    hr = _device->CreateBuffer(&desc, &initial_data, &_buffer);

  } else {
    hr = _device->CreateBuffer(&desc, nullptr, &_buffer);
  }

  if (FAILED(hr)) {
    return false;
  }

  nassertr(_buffer != nullptr, false);

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Created D3D11 buffer, size " << size << ", usage " << usage << ", client pointer " << (void *)client_pointer << "\n";
  }

  return true;
}

/**
 * Updates the buffer with new data from client_pointer.  Assumes the buffer
 * has already been created, and the size of the memory block referred to by
 * client_pointer is at least the size of the buffer.
 */
bool DXBufferBase11::
update_buffer(ID3D11DeviceContext *context, const unsigned char *client_pointer) {
  if (_buffer == nullptr || client_pointer == nullptr) {
    return false;
  }

  if (_dynamic) {
    if (dxgsg11_cat.is_debug()) {
      dxgsg11_cat.debug()
        << "Map()/Unmap() on dynamic buffer, client_pointer " << (void *)client_pointer << "\n";
    }

    // The D3D11 documentation recommends using Map()/Unmap() for buffers
    // that change once or more per frame.  I'm going to assume that
    // UH_dynamic and UH_stream buffers do that.
    D3D11_MAPPED_SUBRESOURCE mapped_data;
    HRESULT hr = context->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
    if (SUCCEEDED(hr)) {
      memcpy(mapped_data.pData, client_pointer, _buffer_size);
      context->Unmap(_buffer, 0);

      return true;

    } else {
      dxgsg11_cat.error()
        << "Failed to map dynamic buffer for updating: " << hr << "\n";

      return false;
    }

  } else {
    if (dxgsg11_cat.is_debug()) {
      dxgsg11_cat.debug()
        << "UpdateSubresource() on non-dynamic buffer, client_pointer " << (void *)client_pointer << "\n";
    }

    // UpdateSubresource() for buffers that change less than once per frame.
    D3D11_BOX box;
    box.back = 1;
    box.front = 0;
    box.bottom = 1;
    box.top = 0;
    box.left = 0;
    box.right = _buffer_size;
    context->UpdateSubresource(_buffer, 0, &box, (void *)client_pointer, 0, 0);

    return true;
  }
}
