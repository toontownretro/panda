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
  DXBufferBase11(gsg->get_device(), D3D11_BIND_VERTEX_BUFFER)
{
  // Make it old so the next call to update_buffer() will create the buffer
  // and fill in the initial data.
  update_modified(UpdateSeq::old());
}

/**
 *
 */
DXVertexBufferContext11::
~DXVertexBufferContext11() {
}

/**
 * Ensures the GPU contains the latest vertex buffer data.
 */
void DXVertexBufferContext11::
update_buffer(ID3D11DeviceContext *context, const GeomVertexArrayDataHandle *reader) {
  if (was_modified(reader)) {
    if (_immutable || changed_size(reader) || changed_usage_hint(reader) || _buffer == nullptr) {
      // Buffer needs to be recreated.  New data will be supplied in the initial data.
      if (dxgsg11_cat.is_debug()) {
        dxgsg11_cat.debug()
          << "Recreating vertex buffer that changed size, usage hint, is immutable, or not yet created\n";
      }

      if (create_buffer(reader->get_usage_hint(), reader->get_data_size_bytes(),
                        reader->get_read_pointer(true))) {
        mark_loaded(reader);
      }

    } else {
      // We can modify the existing data without recreating the buffer.
      if (DXBufferBase11::update_buffer(context, reader->get_read_pointer(true))) {
        mark_loaded(reader);
      }
    }
  }
}
