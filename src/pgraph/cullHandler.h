/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullHandler.h
 * @author drose
 * @date 2002-02-23
 */

#ifndef CULLHANDLER_H
#define CULLHANDLER_H

#include "pandabase.h"
#include "cullableObject.h"
#include "graphicsStateGuardianBase.h"
#include "cullResult.h"
#include "cullTraverser.h"

/**
 * This is an object that receives Geoms from the CullTraverser and takes
 * appropriate action based on the configured handle type.
 */
class EXPCL_PANDA_PGRAPH CullHandler {
public:
  enum HandleType {
    HT_bin, // Collect all of the objects into bins, sort the objects within
            // each bin, and draw the objects within each bin.
    HT_draw, // Draw objects has soon as they are encountered during the
             // Cull traversal.
  };

  INLINE CullHandler(HandleType type, CullResult *result, GraphicsStateGuardianBase *gsg);

  INLINE void record_object(CullableObject *object,
                            const CullTraverser *traverser);
  INLINE void end_traverse();

  INLINE static void draw(CullableObject *object,
                          GraphicsStateGuardianBase *gsg,
                          bool force, Thread *current_thread);

private:
  HandleType _type;
  CullResult *_result;
  GraphicsStateGuardianBase *_gsg;
};

#include "cullHandler.I"

#endif
