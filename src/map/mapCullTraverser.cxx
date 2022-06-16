/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mapCullTraverser.cxx
 * @author brian
 * @date 2021-10-15
 */

#include "mapCullTraverser.h"
#include "cullTraverserData.h"
#include "pandaNode.h"
#include "mapData.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "lightAttrib.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "shaderAttrib.h"

IMPLEMENT_CLASS(MapCullTraverser);

/**
 *
 */
MapCullTraverser::
MapCullTraverser(const CullTraverser &copy, MapData *data) :
  CullTraverser(copy),
  _data(data),
  _view_cluster(-1)
{
}

/**
 *
 */
void MapCullTraverser::
determine_view_cluster() {
  _view_cluster = -1;
  _pvs.clear();

  if (_data == nullptr) {
    return;
  }

  const SpatialPartition *tree = _data->get_area_cluster_tree();
  if (tree == nullptr) {
    return;
  }

  if (_scene_setup->get_camera_node()->get_pvs_cull()) {
    LPoint3 view_pos = get_camera_transform()->get_pos();
    _view_cluster = tree->get_leaf_value_from_point(view_pos);
    if (_view_cluster != -1) {
      _pvs.set_bit(_view_cluster);
      const AreaClusterPVS *pvs = _data->get_cluster_pvs(_view_cluster);
      for (size_t i = 0; i < pvs->get_num_visible_clusters(); i++) {
        _pvs.set_bit(pvs->get_visible_cluster(i));
      }
    }
  }
}
