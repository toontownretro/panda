/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsStateGuardian11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef DXGRAPHICSSTATEGUARDIAN11_H
#define DXGRAPHICSSTATEGUARDIAN11_H

#include "pandabase.h"
#include "graphicsStateGuardian.h"
#include "geomMunger.h"
#include "pointerTo.h"

#include <pmap.h>

class ID3D11Device;
class ID3D11DeviceContext;

/**
 * There is one DXGraphicsStateGuardian for each DXGraphicsDevice being used.
 */
class EXPCL_PANDA_DXGSG11 DXGraphicsStateGuardian11 : public GraphicsStateGuardian {
public:
  DXGraphicsStateGuardian11(GraphicsEngine *engine, GraphicsPipe *pipe,
                            ID3D11Device *device, ID3D11DeviceContext *context);

  virtual PT(GeomMunger) make_geom_munger(const RenderState *state, Thread *current_thread) override;

  INLINE ID3D11Device *get_device() const;
  INLINE ID3D11DeviceContext *get_context() const;

  virtual bool begin_frame(Thread *current_thread) override;
  virtual bool prepare_lens() override;

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr) override;

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data) override;
  virtual void release_vertex_buffer(VertexBufferContext *vbc) override;

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data) override;
  virtual void release_index_buffer(IndexBufferContext *ibc) override;

private:
  ID3D11Device *_device;
  ID3D11DeviceContext *_context;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "DXGraphicsStateGuardian11",
                  GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGraphicsStateGuardian11.I"

#endif // DXGRAPHICSSTATEGUARDIAN11_H
