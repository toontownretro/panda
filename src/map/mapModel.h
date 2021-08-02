/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapModel.h
 * @author brian
 * @date 2021-07-09
 */

#ifndef MAPMODEL_H
#define MAPMODEL_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"

/**
 * A model in a map is a collection of map polygons associated with a
 * particular entity.
 */
class EXPCL_PANDA_MAP MapModel : public TypedWritableReferenceCount {
  DECLARE_CLASS(MapModel, TypedWritableReferenceCount);

};

#include "mapModel.I"

#endif // MAPMODEL_H
