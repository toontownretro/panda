/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsBuffer11.h
 * @author brian
 * @date 2022-03-01
 */

#ifndef WDXGRAPHICSBUFFER11_H
#define WDXGRAPHICSBUFFER11_H

#include "pandabase.h"
#include "graphicsBuffer.h"

/**
 *
 */
class EXPCL_PANDA_DXGSG11 wdxGraphicsBuffer11 : public GraphicsBuffer {
public:
  wdxGraphicsBuffer11(GraphicsEngine *engine,
                      GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "wdxGraphicsBuffer11",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wdxGraphicsBuffer11.I"

#endif // WDXGRAPHICSBUFFER11_H
