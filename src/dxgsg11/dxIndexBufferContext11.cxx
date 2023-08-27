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
#include "geomIndexArrayData.h"
#include "geomVertexArrayData.h"

#include <d3d11.h>

TypeHandle DXIndexBufferContext11::_type_handle;

/**
 *
 */
DXIndexBufferContext11::
DXIndexBufferContext11(DXGraphicsStateGuardian11 *gsg, PreparedGraphicsObjects *pgo,
                       GeomIndexArrayData *data) :
  IndexBufferContext(pgo, data),
  DXBufferBase11(gsg->get_device(), D3D11_BIND_INDEX_BUFFER)
{
  // Make it old so the next call to update_buffer() will create the buffer
  // and fill in the initial data.
  update_modified(UpdateSeq::old());
}

/**
 *
 */
DXIndexBufferContext11::
~DXIndexBufferContext11() {
}

/**
 * Ensures the GPU contains the latest index buffer data.
 */
void DXIndexBufferContext11::
update_buffer(ID3D11DeviceContext *context, const GeomVertexArrayDataHandle *reader) {
  if (was_modified(reader)) {
    if (_immutable || changed_size(reader) || changed_usage_hint(reader) || _buffer == nullptr) {
      // Buffer needs to be recreated.  New data will be supplied in the initial data.
      if (dxgsg11_cat.is_debug()) {
        dxgsg11_cat.debug()
          << "Recreating index buffer that changed size, usage hint, is immutable, or not yet created\n";
      }

      reader->acquire_rw_lock();

      if (create_buffer(reader->get_usage_hint(), reader->get_data_size_bytes(),
                        reader->get_read_pointer(true))) {
        mark_loaded(reader);
      }

    } else {
      reader->acquire_rw_lock();
      // We can modify the existing data without recreating the buffer.
      if (DXBufferBase11::update_buffer(context, reader->get_read_pointer(true))) {
        mark_loaded(reader);
      }
    }
  }
}
