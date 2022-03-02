/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxIndexBufferContext11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXINDEXBUFFERCONTEXT11_H
#define DXINDEXBUFFERCONTEXT11_H

#include "pandabase.h"
#include "indexBufferContext.h"

class GeomPrimitive;
class PreparedGraphicsObjects;
class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11Buffer;
class GeomPrimitivePipelineReader;
class DXGraphicsStateGuardian11;

/**
 *
 */
class EXPCL_PANDA_DXGSG11 DXIndexBufferContext11 : public IndexBufferContext {
public:
  DXIndexBufferContext11(DXGraphicsStateGuardian11 *gsg, PreparedGraphicsObjects *pgo, GeomPrimitive *data);
  virtual ~DXIndexBufferContext11();

  void create_buffer(ID3D11Device *device, const GeomPrimitivePipelineReader *reader);

  void update_buffer(ID3D11Device *device, ID3D11DeviceContext *context,
                     const GeomPrimitivePipelineReader *reader);

  INLINE ID3D11Buffer *get_buffer() const;
  INLINE bool is_immutable() const;

private:
  ID3D11Buffer *_ibuffer;

  // If true, the initial data supplied to the index buffer on creation is
  // final.  The CPU cannot write to the index buffer.  If the index data
  // is modified, the index buffer must be torn down and recreated.
  bool _immutable;

  // If true, the buffer was created with UH_dynamic/UH_stream usage and is
  // assumed that the data will change at least once per frame.
  bool _dynamic;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IndexBufferContext::init_type();
    register_type(_type_handle, "DXIndexBufferContext11",
                  IndexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxIndexBufferContext11.I"

#endif // DXINDEXBUFFERCONTEXT11_H
