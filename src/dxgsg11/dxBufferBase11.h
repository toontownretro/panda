/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxBufferBase11.h
 * @author brian
 * @date 2022-03-02
 */

#ifndef DXBUFFERBASE11_H
#define DXBUFFERBASE11_H

#include "pandabase.h"
#include "geomEnums.h"

#include <d3d11.h>

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXBufferBase11 {
public:
  DXBufferBase11(ID3D11Device *device, D3D11_BIND_FLAG bind_flags);
  virtual ~DXBufferBase11();

  bool create_buffer(GeomEnums::UsageHint usage, size_t size,
                     const unsigned char *client_pointer);

  bool update_buffer(ID3D11DeviceContext *context,
                     const unsigned char *client_pointer);

  INLINE ID3D11Buffer *get_buffer() const;

protected:
  ID3D11Device *_device;
  ID3D11Buffer *_buffer;
  // If true, the initial data supplied to the buffer on creation is
  // final.  The CPU cannot write to the buffer.  If the data
  // is modified, the buffer must be torn down and recreated.
  bool _immutable;
  // If true, the buffer was created with UH_dynamic/UH_stream usage and is
  // assumed that the data will change at least once per frame.
  bool _dynamic;

  size_t _buffer_size;

  D3D11_BIND_FLAG _bind_flags;
};

#include "dxBufferBase11.I"

#endif // DXBUFFERBASE11_H
