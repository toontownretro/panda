/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxIndexBufferContext11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxIndexBufferContext11.h"
#include "geomPrimitive.h"
#include "dxGraphicsStateGuardian11.h"
#include "config_dxgsg11.h"

#include <d3d11.h>

TypeHandle DXIndexBufferContext11::_type_handle;

/**
 *
 */
DXIndexBufferContext11::
DXIndexBufferContext11(DXGraphicsStateGuardian11 *gsg, PreparedGraphicsObjects *pgo,
                       GeomPrimitive *data) :
  IndexBufferContext(pgo, data),
  _immutable(false),
  _ibuffer(nullptr),
  _dynamic(false) {

  GeomPrimitivePipelineReader reader(data, Thread::get_current_thread());
  create_buffer(gsg->get_device(), &reader);
}

/**
 *
 */
void DXIndexBufferContext11::
create_buffer(ID3D11Device *device, const GeomPrimitivePipelineReader *reader) {
  if (_ibuffer != nullptr) {
    // Release existing buffer.
    _ibuffer->Release();
    _ibuffer = nullptr;
  }

  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));

  switch (reader->get_usage_hint()) {
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
  desc.ByteWidth = reader->get_data_size_bytes();
  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

  // Fill in initial data.  If the buffer is immutable, this is the final
  // data.
  D3D11_SUBRESOURCE_DATA initial_data;
  initial_data.pSysMem = (const void *)reader->get_read_pointer(true);
  initial_data.SysMemPitch = 0;
  initial_data.SysMemSlicePitch = 0;

  // Create the index buffer!
  HRESULT hr = device->CreateBuffer(&desc, &initial_data, &_ibuffer);
  if (FAILED(hr)) {
    dxgsg11_cat.error()
      << "Failed to create index buffer: " << hr << "\n";
  }

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Created index buffer, size " << reader->get_data_size_bytes() << ", usage " << reader->get_usage_hint() << "\n";
  }

  mark_loaded(reader);
}

/**
 * Ensures the GPU contains the latest index buffer data.
 */
void DXIndexBufferContext11::
update_buffer(ID3D11Device *device, ID3D11DeviceContext *context, const GeomPrimitivePipelineReader *reader) {
  if (was_modified(reader)) {

    if (_immutable || changed_size(reader) || changed_usage_hint(reader)) {
      // Buffer needs to be recreated.  New data will be supplied in the initial data.
      if (dxgsg11_cat.is_debug()) {
        dxgsg11_cat.debug()
          << "Recreating index buffer that changed size, usage hint, or is immutable\n";
      }
      create_buffer(device, reader);

    } else {
      // We can modify the existing data without recreating the buffer.

      if (_dynamic) {
        if (dxgsg11_cat.is_debug()) {
          dxgsg11_cat.debug()
            << "Mapping dynamic index buffer for update\n";
        }

        // The D3D11 documentation recommends using Map()/Unmap() for buffers
        // that change once or more per frame.  I'm going to assume that
        // UH_dynamic and UH_stream buffers do that.
        D3D11_MAPPED_SUBRESOURCE mapped_data;
        HRESULT hr = context->Map(_ibuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        if (SUCCEEDED(hr)) {
          mapped_data.pData = (void *)reader->get_read_pointer(true);
          context->Unmap(_ibuffer, 0);
          mark_loaded(reader);

        } else {
          dxgsg11_cat.error()
            << "Failed to map dynamic index buffer for updating: " << hr << "\n";
        }

      } else {
        if (dxgsg11_cat.is_debug()) {
          dxgsg11_cat.debug()
            << "UpdateSubresource() on non-dynamic index buffer\n";
        }

        // UpdateSubresource() for buffers that change less than once per frame.
        D3D11_BOX box;
        box.back = 1;
        box.front = 0;
        box.bottom = 1;
        box.top = 0;
        box.left = 0;
        box.right = reader->get_data_size_bytes();
        context->UpdateSubresource(_ibuffer, 0, &box, (void *)reader->get_read_pointer(true), 0, 0);
        mark_loaded(reader);
      }
    }
  }
}

/**
 *
 */
DXIndexBufferContext11::
~DXIndexBufferContext11() {
  if (_ibuffer != nullptr) {
    _ibuffer->Release();
  }
}
