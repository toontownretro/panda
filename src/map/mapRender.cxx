/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapRender.cxx
 * @author brian
 * @date 2021-10-15
 */

#include "mapRender.h"
#include "mapData.h"
#include "mapCullTraverser.h"

IMPLEMENT_CLASS(MapRender);

/**
 *
 */
MapRender::
MapRender(const std::string &name) :
  PandaNode(name),
  _map_data(nullptr)
{
  set_cull_callback();
  set_renderable();
}

/**
 *
 */
bool MapRender::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  if (_map_data == nullptr) {
    return true;
  }

  MapCullTraverser mtrav(*trav, _map_data);
  mtrav.local_object();
  mtrav.determine_view_cluster();
  mtrav.traverse_below(data);
  mtrav.end_traverse();

  // The MapCullTraverser has taken care of the traversal for this node and
  // below.
  return false;
}
