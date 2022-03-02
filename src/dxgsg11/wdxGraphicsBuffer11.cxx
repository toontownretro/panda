/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsBuffer11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "wdxGraphicsBuffer11.h"

TypeHandle wdxGraphicsBuffer11::_type_handle;

/**
 *
 */
wdxGraphicsBuffer11::
wdxGraphicsBuffer11(GraphicsEngine *engine,
                    GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
}
