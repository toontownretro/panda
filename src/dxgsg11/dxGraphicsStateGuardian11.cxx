/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsStateGuardian11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "dxGraphicsStateGuardian11.h"
#include "coordinateSystem.h"
#include "dxGeomMunger11.h"
#include "dxVertexBufferContext11.h"
#include "dxIndexBufferContext11.h"

#include <d3d11.h>

TypeHandle DXGraphicsStateGuardian11::_type_handle;

/**
 *
 */
DXGraphicsStateGuardian11::
DXGraphicsStateGuardian11(GraphicsEngine *engine, GraphicsPipe *pipe,
                          ID3D11Device *device, ID3D11DeviceContext *context) :
  GraphicsStateGuardian(CS_zup_right, engine, pipe),
  _device(device),
  _context(context)
{
}

/**
 * Creates a new GeomMunger object to munge vertices appropriate to this GSG
 * for the indicated state.
 */
PT(GeomMunger) DXGraphicsStateGuardian11::
make_geom_munger(const RenderState *state, Thread *current_thread) {
  PT(DXGeomMunger11) munger = new DXGeomMunger11(this, state);
  return GeomMunger::register_munger(munger, current_thread);
}

/**
 * Called before each frame is rendered, to allow the GSG a chance to do any
 * internal cleanup before beginning the frame.
 *
 * The return value is true if successful (in which case the frame will be
 * drawn and end_frame() will be called later), or false if unsuccessful (in
 * which case nothing will be drawn and end_frame() will not be called).
 */
bool DXGraphicsStateGuardian11::
begin_frame(Thread *current_thread) {
  return GraphicsStateGuardian::begin_frame(current_thread);
}

/**
 * Makes the current lens (whichever lens was most recently specified with
 * set_scene()) active, so that it will transform future rendered geometry.
 * Normally this is only called from the draw process, and usually it is
 * called by set_scene().
 *
 * The return value is true if the lens is acceptable, false if it is not.
 */
bool DXGraphicsStateGuardian11::
prepare_lens() {
  // No-op.  Projection matrix is a shader constant.
  return true;
}

/**
 * Makes the specified DisplayRegion current.  All future drawing and clear
 * operations will be constrained within the given DisplayRegion.
 */
void DXGraphicsStateGuardian11::
prepare_display_region(DisplayRegionPipelineReader *dr) {
  GraphicsStateGuardian::prepare_display_region(dr);

  int count = dr->get_num_regions();
  bool do_scissor = dr->get_scissor_enabled();
  PN_stdfloat near_depth, far_depth;
  dr->get_depth_range(near_depth, far_depth);

  D3D11_VIEWPORT *vp = (D3D11_VIEWPORT *)alloca(sizeof(D3D11_VIEWPORT) * count);
  D3D11_RECT *rect;
  if (do_scissor) {
    rect = (D3D11_RECT *)alloca(sizeof(D3D11_RECT) * count);
  }

  for (int i = 0; i < count; ++i) {
    int xo, yo, w, h;
    dr->get_region_pixels_i(i, xo, yo, w, h);

    vp[i].Width = w;
    vp[i].Height = h;
    vp[i].TopLeftX = xo;
    vp[i].TopLeftY = yo;
    vp[i].MinDepth = near_depth;
    vp[i].MaxDepth = far_depth;

    if (do_scissor) {
      rect[i].left = xo;
      rect[i].top = yo;
      rect[i].right = w + xo;
      rect[i].bottom = h + yo;
    }
  }

  _context->RSSetViewports(count, vp);

  if (do_scissor) {
    _context->RSSetScissorRects(count, rect);
  }
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
VertexBufferContext *DXGraphicsStateGuardian11::
prepare_vertex_buffer(GeomVertexArrayData *data) {
  DXVertexBufferContext11 *dvbc = new DXVertexBufferContext11(this, _prepared_objects, data);
  return dvbc;
}

/**
 *
 */
void DXGraphicsStateGuardian11::
release_vertex_buffer(VertexBufferContext *vbc) {
  DXVertexBufferContext11 *dvbc = DCAST(DXVertexBufferContext11, vbc);
  delete dvbc;
}

/**
 * Prepares the indicated buffer for retained-mode rendering.
 */
IndexBufferContext *DXGraphicsStateGuardian11::
prepare_index_buffer(GeomPrimitive *prim) {
  DXIndexBufferContext11 *dibc = new DXIndexBufferContext11(this, _prepared_objects, prim);
  return dibc;
}

/**
 *
 */
void DXGraphicsStateGuardian11::
release_index_buffer(IndexBufferContext *ibc) {
  DXIndexBufferContext11 *dibc = DCAST(DXIndexBufferContext11, ibc);
  delete dibc;
}
