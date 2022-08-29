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
#include "sceneSetup.h"

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
}

/**
 *
 */
bool MapRender::
cull_callback(CullTraverser *trav, CullTraverserData &data) {

  SceneSetup *scene = trav->get_scene();
  LPoint3 pos;

  // The user might want the PVS to be determined from a different node from
  // the camera itself.  For instance, the reflection camera should use the
  // same PVS as the main camera.  The reflection camera is flipped underground
  // and most likely in solid space.
  CameraPVSCenters::const_iterator it = _pvs_centers.find(scene->get_camera_node());
  if (it == _pvs_centers.end()) {
    pos = scene->get_camera_transform()->get_pos();
  } else {
    pos = (*it).second.get_pos(scene->get_scene_root());
  }

  MapCullTraverser mtrav(*trav, _map_data);
  mtrav.local_object();
  mtrav.determine_view_cluster(pos);
  mtrav.traverse_below(data);
  mtrav.end_traverse();

  // The MapCullTraverser has taken care of the traversal for this node and
  // below.
  return false;
}
