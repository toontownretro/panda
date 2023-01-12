/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderMangerBase.h
 * @author brian
 * @date 2020-11-03
 */

#ifndef SHADERMANAGERBASE_H
#define SHADERMANAGERBASE_H

#include "pandabase.h"
#include "pointerTo.h"

class RenderAttrib;
class RenderState;
class GeomVertexAnimationSpec;
class GraphicsStateGuardianBase;

/**
 * Abstract interface class to the ShaderManager defined in shader.  This is
 * needed to avoid having display depend on shader.
 */
class EXPCL_PANDA_GSGBASE ShaderManagerBase {
public:
  virtual CPT(RenderAttrib) generate_shader(GraphicsStateGuardianBase *gsg,
                                            const RenderState *state,
                                            const GeomVertexAnimationSpec &anim_spec)=0;

  static INLINE void set_global_shader_manager(ShaderManagerBase *mgr);
  static INLINE ShaderManagerBase *get_global_shader_manager();

private:
  static ShaderManagerBase *_global_mgr;
};

#include "shaderManagerBase.I"

#endif // SHADEREMANAGERBASE_H
