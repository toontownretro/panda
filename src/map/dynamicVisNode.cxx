/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicVisNode.cxx
 * @author brian
 * @date 2021-12-03
 */

#include "dynamicVisNode.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "clockObject.h"
#include "mapCullTraverser.h"
#include "mapData.h"
#include "spatialPartition.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "omniBoundingVolume.h"

IMPLEMENT_CLASS(DynamicVisNode);

/**
 * xyz
 */
DynamicVisNode::
DynamicVisNode(const std::string &name) :
  PandaNode(name),
  _trav_counter(-1),
  _last_visit_frame(-1),
  _enabled(true)
{
  // Give it infinite bounds to optimize recomputing the node's bounding
  // volume when we have a bunch of children.
  set_bounds(new OmniBoundingVolume);
  // This indicates cull_callback() should be called on this node when it is
  // visited during the Cull traversal.  The cull callback will traverse
  // the children in buckets of visgroups in the PVS.
  set_cull_callback();
}

/**
 *
 */
void DynamicVisNode::
set_culling_enabled(bool flag) {
  _enabled = flag;
}

/**
 *
 */
bool DynamicVisNode::
get_culling_enabled() const {
  return _enabled;
}

/**
 * Called when a new level has been loaded.  It makes sure there are buckets
 * for each visgroup in the new level.
 */
void DynamicVisNode::
level_init(int num_clusters) {
  // Everything else should've been reset in level_shutdown().
  _visgroups.resize(num_clusters);

  // Make sure all existing children are in the dirty list.
  for (auto it = _children.begin(); it != _children.end(); ++it) {
    _dirty_children.insert(it->second);
  }
}

/**
 * Called when the current level is being unloaded.  Makes sure all visgroup
 * info for each child is cleared out and the buckets are removed.
 */
void DynamicVisNode::
level_shutdown() {
  // Clear out the visgroup set for each child and any tracking info.
  for (auto it = _children.begin(); it != _children.end(); ++it) {
    it->second->_visgroups.clear();
    it->second->_last_trav_counter = -1;
  }
  _trav_counter = -1;
  _last_visit_frame = -1;
  _visgroups.clear();
  _dirty_children.clear();
}

/**
 * Called when the indicated PandaNode has been added as a child of this node.
 */
void DynamicVisNode::
child_added(PandaNode *node) {
  auto it = _children.find(node);
  if (it != _children.end()) {
    // Hmm, we already have this child in our registry.
    // Mark it dirty just to be safe.
    _dirty_children.insert(it->second);
    return;
  }

  ChildInfo *info = new ChildInfo;
  info->_last_trav_counter = -1;
  info->_node = node;
  _children.insert({ node, info });

  _dirty_children.insert(info);
}

/**
 * Called when the indicated PandaNode has been removed from this node's list
 * of children.
 */
void DynamicVisNode::
child_removed(PandaNode *node) {
  auto it = _children.find(node);
  if (it == _children.end()) {
    return;
  }

  ChildInfo *info = it->second;
  _dirty_children.erase(info);
  remove_from_tree(info);
  _children.erase(it);
}


/**
 * Called when the indicated child node's bounds have been marked as stale.
 */
void DynamicVisNode::
child_bounds_stale(PandaNode *node) {
  auto it = _children.find(node);
  if (it != _children.end()) {
    _dirty_children.insert(it->second);
  }
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool DynamicVisNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {

  MapCullTraverser *mtrav = (MapCullTraverser *)trav;

  if (!_enabled || mtrav->_data == nullptr || mtrav->_view_cluster < 0) {
    // No map, invalid view cluster, or culling disabled.
    return true;
  }

  ClockObject *clock = ClockObject::get_global_clock();
  int frame = clock->get_frame_count();

  // We only need to check for dirty children once per frame.
  // If multiple render passes are done over the scene graph, we assume
  // that nodes will not change during those passes.
  if (frame != _last_visit_frame) {
    _last_visit_frame = frame;

    if (!_dirty_children.empty()) {
      // Re-place all the dirty children into visgroup buckets.
      for (auto it = _dirty_children.begin(); it != _dirty_children.end(); ++it) {
        ChildInfo *info = *it;

        // The bounding volume is in the coordinate space of its parent, meaning
        // it contains the node's local transform already, so we only have to
        // check for a bounding volume change.
        const GeometricBoundingVolume *bounds = (const GeometricBoundingVolume *)info->_node->get_bounds().p();

        // Don't worry about transforming the bounding volume of the node.
        // It is assumed that the DynamicVisNode always has an identity
        // transform and child nodes of it are in world-space already.
        remove_from_tree(info);
        insert_into_tree(info, bounds, mtrav->_data->get_area_cluster_tree());
      }

      _dirty_children.clear();
    }
  }

  _trav_counter++;

  const BitArray &pvs = mtrav->_pvs;
  int num_visgroups = mtrav->_data->get_num_clusters();

  for (int i = 0; i < num_visgroups; ++i) {
    if (!pvs.get_bit(i)) {
      // Not in PVS.
      continue;
    }

    // This visgroup is in the PVS, so traverse the children in the
    // bucket corresponding to this visgroup.
    const DynamicVisNode::ChildSet &children = _visgroups[i];

    for (auto it = children.begin(); it != children.end(); ++it) {
      ChildInfo *child = *it;
      if (child->_last_trav_counter != _trav_counter) {
        child->_last_trav_counter = _trav_counter;
        // We haven't traversed this child yet.
        // TODO: Figure out if there's a way we can store DownConnections
        // instead of node pointers to take advantage of bounds being stored
        // on DownConnection.
        trav->traverse_down(data, child->_node);
      }
    }
  }

  // We've handled the traversal for everything below this node.
  return false;
}

/**
 * Removes the child from all visgroup buckets.
 */
void DynamicVisNode::
remove_from_tree(ChildInfo *info) {
  // Iterate over all the visgroup indices and remove the child from that
  // visgroup's bucket.
  for (size_t i = 0; i < info->_visgroups.size(); ++i) {
    _visgroups[info->_visgroups[i]].erase(info);
  }
  info->_visgroups.clear();
}

/**
 * Inserts the child into the buckets of visgroups that the child's
 * bounding volume overlaps with.  It is assumed that the child's visgroup
 * set is already empty.
 */
void DynamicVisNode::
insert_into_tree(ChildInfo *info, const GeometricBoundingVolume *bounds, const SpatialPartition *tree) {
  //nassertv(bounds != nullptr && !bounds->is_infinite());
  info->_visgroups.reserve(128);

  TypeHandle type = bounds->get_type();

  if (type == BoundingBox::get_class_type()) {
    const BoundingBox *bbox = (const BoundingBox *)bounds;
    if (!bbox->is_empty() && !bbox->is_infinite()) {
      const LPoint3 &mins = bbox->get_minq();
      const LPoint3 &maxs = bbox->get_maxq();
      tree->get_leaf_values_containing_box(mins, maxs, info->_visgroups);
    }

  } else if (type == BoundingSphere::get_class_type()) {
    const BoundingSphere *bsphere = (const BoundingSphere *)bounds;
    if (!bsphere->is_empty() && !bsphere->is_infinite()) {
      LPoint3 center = bsphere->get_center();
      PN_stdfloat radius = bsphere->get_radius();
      tree->get_leaf_values_containing_sphere(center, radius, info->_visgroups);
    }

  } else {
    nassert_raise("Bounds type is not box or sphere!");
    return;
  }

  // Now insert the child into all the buckets.
  for (size_t i = 0; i < info->_visgroups.size(); ++i) {
    int visgroup = info->_visgroups[i];
    _visgroups[visgroup].insert(info);
  }
}
