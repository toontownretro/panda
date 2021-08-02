/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bspWorld.cxx
 * @author brian
 * @date 2021-07-04
 */

#include "bspWorld.h"
#include "omniBoundingVolume.h"
#include "bspData.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "transformState.h"

IMPLEMENT_CLASS(BSPWorld);

/**
 *
 */
BSPWorld::
BSPWorld(BSPData *data) :
  ModelNode("world"),
  _bsp_data(data)
{
  _cluster_geom_nodes.resize(data->dvis->num_clusters);
  set_cull_callback();
  set_bounds(new OmniBoundingVolume);
}

/**
 *
 */
BSPWorld::
BSPWorld(const BSPWorld &copy) :
  ModelNode(copy),
  _bsp_data(copy._bsp_data),
  _cluster_geom_nodes(copy._cluster_geom_nodes)
{
}


/**
 *
 */
bool BSPWorld::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  CPT(TransformState) view_net_transform = trav->get_scene()->get_camera_path().get_net_transform();

  int view_leaf = _bsp_data->get_leaf_containing_point(view_net_transform->get_pos());
  const DLeaf &leaf = _bsp_data->dleafs[view_leaf];

  const BSPClusterVisibility *cluster_vis = nullptr;
  if (leaf.cluster >= 0 && leaf.cluster < _bsp_data->dvis->num_clusters) {
    cluster_vis = &_bsp_data->cluster_vis[leaf.cluster];
  }

  if (cluster_vis == nullptr || cluster_vis->is_all_visible()) {
    if (bsp_cat.is_debug()) {
      bsp_cat.debug()
        << "Drawing all clusters\n";
    }
    for (size_t i = 0; i < _cluster_geom_nodes.size(); i++) {
      GeomNode *geom_node = _cluster_geom_nodes[i];
      if (geom_node != nullptr) {
        trav->traverse_child(data, geom_node);
      }
    }

  } else {
    GeomNode *my_geom_node = _cluster_geom_nodes[cluster_vis->get_cluster_index()];
    if (my_geom_node != nullptr) {
      if (bsp_cat.is_debug()) {
        bsp_cat.debug()
          << "Drawing local cluster " << cluster_vis->get_cluster_index() << "\n";
      }
      trav->traverse_child(data, my_geom_node);
    }
    for (int i = 0; i < cluster_vis->get_num_visible_clusters(); i++) {
      int cluster = cluster_vis->get_visible_cluster(i);
      GeomNode *geom_node = _cluster_geom_nodes[cluster];
      if (geom_node != nullptr) {
        if (bsp_cat.is_debug()) {
          bsp_cat.debug()
            << "Drawing potentially visible cluster " << cluster << "\n";
        }
        trav->traverse_child(data, geom_node);
      }
    }
  }

  return true;
}

/**
 *
 */
bool BSPWorld::
is_renderable() const {
  return true;
}

/**
 *
 */
bool BSPWorld::
safe_to_flatten() const {
  return false;
}

/**
 *
 */
bool BSPWorld::
safe_to_combine() const {
  return false;
}

/**
 *
 */
PandaNode *BSPWorld::
make_copy() const {
  return new BSPWorld(*this);
}
