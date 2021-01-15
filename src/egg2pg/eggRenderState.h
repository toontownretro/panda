/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRenderState.h
 * @author drose
 * @date 2005-03-12
 */

#ifndef EGGRENDERSTATE_H
#define EGGRENDERSTATE_H

#include "pandabase.h"

#include "eggUserData.h"
#include "eggLoader.h"
#include "renderState.h"
#include "renderAttrib.h"
#include "internalName.h"
#include "geomPrimitive.h"
#include "luse.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"

class EggPrimitive;
class EggTexture;

/**
 * This class is used within this package only to record the render state that
 * should be assigned to each primitive.  It is assigned to EggPrimitive
 * objects via the EggBinner.
 */
class EXPCL_PANDA_EGG2PG EggRenderState : public EggUserData {
public:
  INLINE EggRenderState(EggLoader &loader);
  INLINE void add_attrib(const RenderAttrib *attrib);

  void fill_state(EggPrimitive *egg_prim);

  int compare_to(const EggRenderState &other) const;

public:
  CPT(RenderState) _state;
  bool _hidden;
  bool _flat_shaded;
  Geom::PrimitiveType _primitive_type;

private:
  EggLoader &_loader;
};

#include "eggRenderState.I"

#endif
