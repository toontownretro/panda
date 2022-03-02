/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxVertexBufferContext11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXVERTEXBUFFERCONTEXT11_H
#define DXVERTEXBUFFERCONTEXT11_H

#include "pandabase.h"
#include "vertexBufferContext.h"

class ID3D11Buffer;
class ID3D11Device;
class ID3D11DeviceContext;
class PreparedGraphicsObjects;
class GeomVertexArrayData;
class DXGraphicsStateGuardian11;
class GeomVertexArrayDataHandle;

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXVertexBufferContext11 : public VertexBufferContext {
public:
  DXVertexBufferContext11(DXGraphicsStateGuardian11 *gsg, PreparedGraphicsObjects *pgo, GeomVertexArrayData *data);
  virtual ~DXVertexBufferContext11();

  void create_buffer(ID3D11Device *device, const GeomVertexArrayDataHandle *reader);

  void update_buffer(ID3D11Device *device, ID3D11DeviceContext *context,
                     const GeomVertexArrayDataHandle *reader);

  INLINE ID3D11Buffer *get_buffer() const;
  INLINE bool is_immutable() const;

private:
  ID3D11Buffer *_vbuffer;

  // If true, the initial data supplied to the vertex buffer on creation is
  // final.  The CPU cannot write to the vertex buffer.  If the vertex data
  // is modified, the vertex buffer must be torn down and recreated.
  bool _immutable;

  // If true, the buffer was created with UH_dynamic/UH_stream usage and is
  // assumed that the data will change at least once per frame.
  bool _dynamic;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexBufferContext::init_type();
    register_type(_type_handle, "DXVertexBufferContext11",
                  VertexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxVertexBufferContext11.I"

#endif // DXVERTEXBUFFERCONTEXT11_H
