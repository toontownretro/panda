/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxVertexBufferContext11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxVertexBufferContext11.h"
#include "preparedGraphicsObjects.h"
#include "dxGraphicsStateGuardian11.h"
#include "geomVertexArrayData.h"
#include "config_dxgsg11.h"

#include <d3d11.h>

TypeHandle DXVertexBufferContext11::_type_handle;

/**
 *
 */
DXVertexBufferContext11::
DXVertexBufferContext11(DXGraphicsStateGuardian11 *gsg, PreparedGraphicsObjects *pgo,
                        GeomVertexArrayData *data) :
  VertexBufferContext(pgo, data),
  _immutable(false),
  _vbuffer(nullptr),
  _dynamic(false) {

  CPT(GeomVertexArrayDataHandle) reader = data->get_handle();
  create_buffer(gsg->get_device(), reader);
}

/**
 *
 */
void DXVertexBufferContext11::
create_buffer(ID3D11Device *device, const GeomVertexArrayDataHandle *reader) {
  if (_vbuffer != nullptr) {
    // Release existing buffer.
    _vbuffer->Release();
    _vbuffer = nullptr;
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
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  // Fill in initial data.  If the buffer is immutable, this is the final
  // data.
  D3D11_SUBRESOURCE_DATA initial_data;
  initial_data.pSysMem = reader->get_read_pointer(true);
  initial_data.SysMemPitch = 0;
  initial_data.SysMemSlicePitch = 0;

  // Create the vertex buffer!
  HRESULT hr = device->CreateBuffer(&desc, &initial_data, &_vbuffer);
  if (FAILED(hr)) {
    dxgsg11_cat.error()
      << "Failed to create vertex buffer: " << hr << "\n";
  }

  if (dxgsg11_cat.is_debug()) {
    dxgsg11_cat.debug()
      << "Created vertex buffer, size " << reader->get_data_size_bytes() << ", usage " << reader->get_usage_hint() << "\n";
  }

  mark_loaded(reader);
}

/**
 * Ensures the GPU contains the latest vertex buffer data.
 */
void DXVertexBufferContext11::
update_buffer(ID3D11Device *device, ID3D11DeviceContext *context, const GeomVertexArrayDataHandle *reader) {
  if (was_modified(reader)) {

    if (_immutable || changed_size(reader) || changed_usage_hint(reader)) {
      // Buffer needs to be recreated.  New data will be supplied in the initial data.
      if (dxgsg11_cat.is_debug()) {
        dxgsg11_cat.debug()
          << "Recreating vertex buffer that changed size, usage hint, or is immutable\n";
      }
      create_buffer(device, reader);

    } else {
      // We can modify the existing data without recreating the buffer.

      if (_dynamic) {
        if (dxgsg11_cat.is_debug()) {
          dxgsg11_cat.debug()
            << "Mapping dynamic vertex buffer for update\n";
        }

        // The D3D11 documentation recommends using Map()/Unmap() for buffers
        // that change once or more per frame.  I'm going to assume that
        // UH_dynamic and UH_stream buffers do that.
        D3D11_MAPPED_SUBRESOURCE mapped_data;
        HRESULT hr = context->Map(_vbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        if (SUCCEEDED(hr)) {
          mapped_data.pData = (void *)reader->get_read_pointer(true);
          context->Unmap(_vbuffer, 0);
          mark_loaded(reader);

        } else {
          dxgsg11_cat.error()
            << "Failed to map dynamic vertex buffer for updating: " << hr << "\n";
        }

      } else {
        if (dxgsg11_cat.is_debug()) {
          dxgsg11_cat.debug()
            << "UpdateSubresource() on non-dynamic vertex buffer\n";
        }

        // UpdateSubresource() for buffers that change less than once per frame.
        D3D11_BOX box;
        box.back = 1;
        box.front = 0;
        box.bottom = 1;
        box.top = 0;
        box.left = 0;
        box.right = reader->get_data_size_bytes();
        context->UpdateSubresource(_vbuffer, 0, &box, (void *)reader->get_read_pointer(true), 0, 0);
        mark_loaded(reader);
      }
    }
  }
}

/**
 *
 */
DXVertexBufferContext11::
~DXVertexBufferContext11() {
  if (_vbuffer != nullptr) {
    _vbuffer->Release();
  }
}
