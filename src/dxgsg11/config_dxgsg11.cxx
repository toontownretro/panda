/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dxgsg11.cxx
 * @author brian
 * @date 2022-03-01
 */

#include "config_dxgsg11.h"

#include "graphicsPipeSelection.h"
#include "pandaSystem.h"

#include "dxGeomMunger11.h"
#include "dxGraphicsDevice11.h"
#include "dxGraphicsStateGuardian11.h"
#include "dxIndexBufferContext11.h"
#include "dxShaderContext11.h"
#include "dxTextureContext11.h"
#include "dxVertexBufferContext11.h"

#include "wdxGraphicsBuffer11.h"
#include "wdxGraphicsPipe11.h"
#include "wdxGraphicsWindow11.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_DXGSG11)
  #error Buildsystem error: BUILDING_PANDA_DXGSG11 not defined
#endif

NotifyCategoryDef(dxgsg11, ":display:gsg");

ConfigureDef(config_dxgsg11);
ConfigureFn(config_dxgsg11) {
  init_libdxgsg11();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdxgsg11() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  DXGeomMunger11::init_type();
  DXGraphicsDevice11::init_type();
  DXGraphicsStateGuardian11::init_type();
  DXIndexBufferContext11::init_type();
  DXShaderContext11::init_type();
  DXTextureContext11::init_type();
  DXVertexBufferContext11::init_type();

  wdxGraphicsBuffer11::init_type();
  wdxGraphicsPipe11::init_type();
  wdxGraphicsWindow11::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wdxGraphicsPipe11::get_class_type(),
                           wdxGraphicsPipe11::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("DirectX11");
}
