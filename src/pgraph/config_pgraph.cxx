// Filename: config_pgraph.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_pgraph.h"

#include "billboardEffect.h"
#include "qpcamera.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "colorWriteAttrib.h"
#include "cullFaceAttrib.h"
#include "cullBin.h"
#include "cullBinAttrib.h"
#include "cullBinBackToFront.h"
#include "cullBinUnsorted.h"
#include "qpcullTraverser.h"
#include "cullableObject.h"
#include "decalEffect.h"
#include "depthOffsetAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "qpfog.h"
#include "fogAttrib.h"
#include "qpgeomNode.h"
#include "qplensNode.h"
#include "qplodNode.h"
#include "materialAttrib.h"
#include "qpnodePath.h"
#include "qpnodePathComponent.h"
#include "pandaNode.h"
#include "renderAttrib.h"
#include "renderEffect.h"
#include "renderEffects.h"
#include "renderModeAttrib.h"
#include "renderState.h"
#include "selectiveChildNode.h"
#include "qpsequenceNode.h"
#include "texMatrixAttrib.h"
#include "textureApplyAttrib.h"
#include "textureAttrib.h"
#include "transformState.h"
#include "transparencyAttrib.h"

#include "dconfig.h"

ConfigureDef(config_pgraph);
NotifyCategoryDef(pgraph, "");

ConfigureFn(config_pgraph) {
  init_libpgraph();
}

// Set this true to cause culling to be performed by rendering the
// object in red wireframe, rather than actually culling it.  This
// helps make culling errors obvious.
const bool qpfake_view_frustum_cull = config_pgraph.GetBool("fake-view-frustum-cull", false);

// Set this true to make ambiguous path warning messages generate an
// assertion failure instead of just a warning (which can then be
// trapped with assert-abort).
const bool unambiguous_graph = config_pgraph.GetBool("unambiguous-graph", false);

////////////////////////////////////////////////////////////////////
//     Function: init_libpgraph
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  BillboardEffect::init_type();
  qpCamera::init_type();
  ColorAttrib::init_type();
  ColorScaleAttrib::init_type();
  ColorWriteAttrib::init_type();
  CullFaceAttrib::init_type();
  CullBin::init_type();
  CullBinAttrib::init_type();
  CullBinBackToFront::init_type();
  CullBinUnsorted::init_type();
  qpCullTraverser::init_type();
  CullableObject::init_type();
  DecalEffect::init_type();
  DepthOffsetAttrib::init_type();
  DepthTestAttrib::init_type();
  DepthWriteAttrib::init_type();
  qpFog::init_type();
  FogAttrib::init_type();
  qpGeomNode::init_type();
  qpLensNode::init_type();
  qpLODNode::init_type();
  MaterialAttrib::init_type();
  qpNodePath::init_type();
  qpNodePathComponent::init_type();
  PandaNode::init_type();
  RenderAttrib::init_type();
  RenderEffect::init_type();
  RenderEffects::init_type();
  RenderModeAttrib::init_type();
  RenderState::init_type();
  SelectiveChildNode::init_type();
  qpSequenceNode::init_type();
  TexMatrixAttrib::init_type();
  TextureApplyAttrib::init_type();
  TextureAttrib::init_type();
  TransformState::init_type();
  TransparencyAttrib::init_type();

  BillboardEffect::register_with_read_factory();
  qpCamera::register_with_read_factory();
  ColorAttrib::register_with_read_factory();
  ColorScaleAttrib::register_with_read_factory();
  ColorWriteAttrib::register_with_read_factory();
  CullBinAttrib::register_with_read_factory();
  CullFaceAttrib::register_with_read_factory();
  DecalEffect::register_with_read_factory();
  DepthOffsetAttrib::register_with_read_factory();
  DepthTestAttrib::register_with_read_factory();
  DepthWriteAttrib::register_with_read_factory();
  qpFog::register_with_read_factory();
  FogAttrib::register_with_read_factory();
  qpGeomNode::register_with_read_factory();
  qpLensNode::register_with_read_factory();
  qpLODNode::register_with_read_factory();
  MaterialAttrib::register_with_read_factory();
  PandaNode::register_with_read_factory();
  RenderEffects::register_with_read_factory();
  RenderModeAttrib::register_with_read_factory();
  RenderState::register_with_read_factory();
  qpSequenceNode::register_with_read_factory();
  TexMatrixAttrib::register_with_read_factory();
  TextureApplyAttrib::register_with_read_factory();
  TextureAttrib::register_with_read_factory();
  TransformState::register_with_read_factory();
  TransparencyAttrib::register_with_read_factory();
}
