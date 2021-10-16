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
  set_custom_is_in_view(true);
}

/**
 *
 */
int MapCullTraverser::
custom_is_in_view(const CullTraverserData &data) {
  if (_view_cluster == -1) {
    return BoundingVolume::IF_all;
  }

  // Test the node against the cluster tree.
  CPT(GeometricBoundingVolume) vol;

  if (!data._net_transform->is_identity()) {
    // The node has a non-identity net transform.  Make a copy of
    // the bounds and transform it into world space.
    PT(GeometricBoundingVolume) gbv = DCAST(GeometricBoundingVolume,
      data.node_reader()->get_bounds()->make_copy());
    gbv->xform(data._net_transform->get_mat());
    vol = gbv;

  } else {
    vol = DCAST(GeometricBoundingVolume, data.node_reader()->get_bounds());
  }

  const KDTree *tree = _data->get_area_cluster_tree();

  if (!tree->is_volume_in_leaf_set(vol, _pvs)) {
    // The node is not within any of the potentially visible clusters
    // from the camera's position.  Cull the node.
    return BoundingVolume::IF_no_intersection;
  }

  // The node is within 1 or more of the potentially visible clusters.  Keep
  // traversing down this node.
  return BoundingVolume::IF_all;
}

/**
 *
 */
void MapCullTraverser::
determine_view_cluster() {
  _view_cluster = -1;
  _pvs.clear();

  const KDTree *tree = _data->get_area_cluster_tree();
  if (tree == nullptr) {
    return;
  }

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
