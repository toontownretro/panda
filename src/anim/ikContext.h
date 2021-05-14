/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikContext.h
 * @author brian
 * @date 2021-05-13
 */

#ifndef IKCONTEXT_H
#define IKCONTEXT_H

#include "pandabase.h"
#include "luse.h"
#include "transformState.h"
#include "character.h"

/**
 *
 */
class EXPCL_PANDA_ANIM IKContext {
PUBLISHED:
  IKContext();

  void init(Character *character, const TransformState *xform, PN_stdfloat time, int frame_counter);

private:
  Character *_char;
  CPT(TransformState) _root_xform;
}

#include "ikContext.I"

#endif // IKCONTEXT_H
