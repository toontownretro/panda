/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapCullTraverser.h
 * @author brian
 * @date 2021-10-15
 */

#ifndef MAPCULLTRAVERSER_H
#define MAPCULLTRAVERSER_H

#include "pandabase.h"
#include "cullTraverser.h"

class MapData;

/**
 * This is a special kind of CullTraverser that is utilized by the map system.
 * Its only purpose is to determine and store the current visgroup of the
 * camera and its associated PVS for later processes to utilize, such as the
 * DynamicVisNode (dynamic model culling) and MapRoot (static world culling).
 */
class EXPCL_PANDA_MAP MapCullTraverser : public CullTraverser {
  DECLARE_CLASS(MapCullTraverser, CullTraverser);

PUBLISHED:
  // The MapCullTraverser must be initialized from an existing CullTraverser
  // instance.  This is done in the MapRender cull callback.
  MapCullTraverser() = delete;
  MapCullTraverser(const CullTraverser &other, MapData *data);

  void determine_view_cluster();

public:
  // What cluster does the camera currently reside in?  Determined before
  // traversal starts.
  int _view_cluster;
  // A bitmask that describes the current potentially visible set.  Potentially
  // visible clusters, including the current view cluster, have their bit set
  // in this BitArray.
  BitArray _pvs;

  MapData *_data;
};


#include "mapCullTraverser.I"

#endif // MAPCULLTRAVERSER_H
