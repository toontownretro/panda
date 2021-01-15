/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRenderState.cxx
 * @author drose
 * @date 2005-03-12
 */

#include "eggRenderState.h"
#include "eggLine.h"
#include "eggPoint.h"
#include "textureAttrib.h"
#include "renderAttrib.h"
#include "texGenAttrib.h"
#include "internalName.h"
#include "eggCurve.h"
#include "eggSurface.h"
#include "renderStatePool.h"
#include "cullBinAttrib.h"
#include "cullFaceAttrib.h"
#include "shadeModelAttrib.h"
#include "transparencyAttrib.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "depthOffsetAttrib.h"
#include "texMatrixAttrib.h"
#include "renderModeAttrib.h"
#include "config_gobj.h"
#include "config_egg2pg.h"


/**
 * Sets up the state as appropriate for the indicated primitive.
 */
void EggRenderState::
fill_state(EggPrimitive *egg_prim) {
#if 0
  switch (vm) {
  case EggRenderMode::VM_hidden:
    _hidden = true;
    break;

  case EggRenderMode::VM_normal:
  default:
    break;
  }
#endif
  _hidden = false;

  if (egg_prim->has_material()) {
    _state = RenderStatePool::load_state(egg_prim->get_material()->get_filename());
  }

  _flat_shaded =
    (egg_flat_shading &&
     egg_prim->get_connected_shading() == EggPrimitive::S_per_face);

  //if (_flat_shaded) {
  //  add_attrib(ShadeModelAttrib::make(ShadeModelAttrib::M_flat));
  //}

  if (egg_prim->is_of_type(EggLine::get_class_type())) {
    _primitive_type = Geom::PT_lines;
    //EggLine *egg_line = DCAST(EggLine, egg_prim);
    //if (egg_line->get_thick() != 1.0) {
    //  add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_unchanged,
    //                                    egg_line->get_thick()));
    //}
  } else if (egg_prim->is_of_type(EggPoint::get_class_type())) {
    _primitive_type = Geom::PT_points;
    //EggPoint *egg_point = DCAST(EggPoint, egg_prim);
    //if (egg_point->get_thick() != 1.0 || egg_point->get_perspective()) {
    //  add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_unchanged,
    //                                    egg_point->get_thick(),
    //                                    egg_point->get_perspective()));
    //}
  } else {
    _primitive_type = Geom::PT_polygons;
  }
}

/**
 * Provides a unique ordering for different EggRenderState objects, so that
 * primitives of similar state can be grouped together by the EggBinner.
 */
int EggRenderState::
compare_to(const EggRenderState &other) const {
  if (_state != other._state) {
    int c = _state->compare_to(*other._state);
    if (c != 0) {
      return c;
    }
  }
  if (_hidden != other._hidden) {
    return (int)_hidden - (int)other._hidden;
  }
  if (_flat_shaded != other._flat_shaded) {
    return (int)_flat_shaded - (int)other._flat_shaded;
  }
  if (_primitive_type != other._primitive_type) {
    return (int)_primitive_type - (int)other._primitive_type;
  }

  return 0;
}
