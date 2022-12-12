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
#include "jobSystem.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

static PStatCollector dvn_trav_pcollector("DynamicVisNode:Traverse");
static PStatCollector dvn_trav_node_pcollector("DynamicVisNode:TraverseNode");

IMPLEMENT_CLASS(DynamicVisNode);

/**
 * xyz
 */
DynamicVisNode::
DynamicVisNode(const std::string &name) :
  PandaNode(name),
  _trav_counter(-1),
  _tree(nullptr)
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
  //CDWriter cdata(_cycler);
  CData *cdata = &_cdata;
  cdata->_enabled = flag;
}

/**
 *
 */
bool DynamicVisNode::
get_culling_enabled() const {
  //CDReader cdata(_cycler);
  const CData *cdata = &_cdata;
  return cdata->_enabled;
}

/**
 *
 */
void DynamicVisNode::
update_dirty_children() {
  if (_tree == nullptr) {
    return;
  }

  nassertv(Thread::get_current_pipeline_stage() == 0);

  JobSystem *jsys = JobSystem::get_global_ptr();

  //Children children = get_children();
  //jsys->parallel_process(children.size(), [&children] (int i) {
  //    PandaNodePipelineReader reader(children.get_child(i), Thread::get_current_thread());
  //    reader.check_cached(true);
  // }
  //);

  if (!_dirty_children.empty()) {
    //CDWriter cdata(_cycler);
    CData *cdata = &_cdata;

    // Re-place all the dirty children into visgroup buckets.
    for (auto it = _dirty_children.begin(); it != _dirty_children.end(); ++it) {
      ChildInfo *info = *it;
      // Don't worry about transforming the bounding volume of the node.
      // It is assumed that the DynamicVisNode always has an identity
      // transform and child nodes of it are in world-space already.
      remove_from_tree(info, cdata);
    }

    jsys->parallel_process(_dirty_children.size(),
      [this] (int i) {
        ChildInfo *info = _dirty_children[i];
        // The bounding volume is in the coordinate space of its parent, meaning
        // it contains the node's local transform already, so we only have to
        // check for a bounding volume change.
        const GeometricBoundingVolume *bounds = (const GeometricBoundingVolume *)info->_node->get_bounds().p();
        insert_into_tree(info, bounds, _tree);

        info->_dirty = false;
      }
    );

    for (auto it = _dirty_children.begin(); it != _dirty_children.end(); ++it) {
      ChildInfo *info = *it;
      // Now insert the child into all the buckets.
      for (size_t i = 0; i < info->_visgroups.size(); ++i) {
        int visgroup = info->_visgroups[i];
        cdata->_visgroups[visgroup].store(info, nullptr);
      }
    }

    _dirty_children.clear();
  }
}

/**
 * Called when a new level has been loaded.  It makes sure there are buckets
 * for each visgroup in the new level.
 */
void DynamicVisNode::
level_init(int num_clusters, const SpatialPartition *tree) {
  _tree = tree;

  //CDWriter cdata(_cycler);
  CData *cdata = &_cdata;

  // Everything else should've been reset in level_shutdown().
  cdata->_visgroups.resize(num_clusters);

  // Make sure all existing children are in the dirty list.
  for (auto it = _children.begin(); it != _children.end(); ++it) {
    ChildInfo *info = (*it).second;
    if (!info->_dirty) {
      info->_dirty = true;
      _dirty_children.push_back(info);
    }
  }
}

/**
 * Called when the current level is being unloaded.  Makes sure all visgroup
 * info for each child is cleared out and the buckets are removed.
 */
void DynamicVisNode::
level_shutdown() {
  //CDWriter cdata(_cycler);
  CData *cdata = &_cdata;

  // Clear out the visgroup set for each child and any tracking info.
  for (auto it = _children.begin(); it != _children.end(); ++it) {
    it->second->_visgroups.clear();
    it->second->_last_trav_counter = -1;
    it->second->_dirty = false;
  }
  _trav_counter = -1;
  _tree = nullptr;
  cdata->_visgroups.clear();
  _dirty_children.clear();
}

/**
 * Called when the indicated PandaNode has been added as a child of this node.
 */
void DynamicVisNode::
child_added(PandaNode *node, int pipeline_stage) {
  // This should not be called from Cull or Draw.
  nassertv(Thread::get_current_pipeline_stage() == 0);

  auto it = _children.find(node);
  if (it != _children.end()) {
    // Hmm, we already have this child in our registry.
    // Mark it dirty just to be safe.
    ChildInfo *info = (*it).second;
    if (!info->_dirty) {
      _dirty_children.push_back(info);
      info->_dirty = true;
    }
    return;
  }

  ChildInfo *info = new ChildInfo;
  info->_last_trav_counter = -1;
  info->_node = node;
  info->_dirty = true;
  _children.insert({ node, info });
  _dirty_children.push_back(info);
}

/**
 * Called when the indicated PandaNode has been removed from this node's list
 * of children.
 */
void DynamicVisNode::
child_removed(PandaNode *node, int pipeline_stage) {
  // This should not be called from Cull or Draw.
  nassertv(Thread::get_current_pipeline_stage() == 0);

  auto it = _children.find(node);
  if (it == _children.end()) {
    return;
  }

  ChildInfo *info = it->second;
  if (info->_dirty) {
    auto dit = std::find(_dirty_children.begin(), _dirty_children.end(), info);
    if (dit != _dirty_children.end()) {
      _dirty_children.erase(dit);
    }
    info->_dirty = false;
  }

  {
    //CDWriter cdata(_cycler);
    remove_from_tree(info, &_cdata);
  }
  _children.erase(it);
}


/**
 * Called when the indicated child node's bounds have been marked as stale.
 */
void DynamicVisNode::
child_bounds_stale(PandaNode *node, int pipeline_stage) {
  // Ideally this is only called from App.
  nassertv(Thread::get_current_pipeline_stage() == 0);
  //if (Thread::get_current_pipeline_stage() != 0) {
  //  return;
  //}

  ChildInfos::const_iterator it = _children.find(node);
  if (it != _children.end()) {
    ChildInfo *info = (*it).second;
    if (!info->_dirty) {
      _dirty_children.push_back(info);
      info->_dirty = true;
    }
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

  //CDReader cdata(_cycler, trav->get_current_thread());
  const CData *cdata = &_cdata;

  if (!cdata->_enabled || mtrav->_data == nullptr) {
    // No map, invalid view cluster, or culling disabled.
    return true;
  }

  if (mtrav->_view_cluster < 0) {
    return false;
  }

  //_trav_counter++;

  PStatTimer timer(dvn_trav_pcollector);

  const BitArray &pvs = mtrav->_pvs;
  //int num_visgroups = mtrav->_data->get_num_clusters();

  SimpleHashMap<ChildInfo *, std::nullptr_t, pointer_hash> traversed;
  //traversed.resize_table()
  //traversed.clear();

  // Iterate over the subset of visgroups in the PVS.
  int first = pvs.get_lowest_on_bit();
  if (first == -1) {
    return false;
  }
  int last = pvs.get_highest_on_bit();
  nassertr(last != -1, false);

  for (int i = first; i <= last; ++i) {
    if (!pvs.get_bit(i)) {
      // Not in PVS.
      continue;
    }

    // This visgroup is in the PVS, so traverse the children in the
    // bucket corresponding to this visgroup.
    const DynamicVisNode::ChildSet &children = cdata->_visgroups[i];

    for (size_t j = 0; j < children.size(); ++j) {
      ChildInfo *child = children.get_key(j);
      if (traversed.find(child) == -1) {
        {
          //PStatTimer timer2(dvn_trav_node_pcollector);
          trav->traverse_down(data, child->_node);
        }
        traversed.store(child, nullptr);
      }
    }

    //for (auto it = children.begin(); it != children.end(); ++it) {
    //  ChildInfo *child = *it;
    // / auto ret = traversed.insert(child);
    //  if (ret.second) {

     // }
    //}
  }

  // We've handled the traversal for everything below this node.
  return false;
}

/**
 * Removes the child from all visgroup buckets.
 */
void DynamicVisNode::
remove_from_tree(ChildInfo *info, CData *cdata) {
  // Iterate over all the visgroup indices and remove the child from that
  // visgroup's bucket.
  for (size_t i = 0; i < info->_visgroups.size(); ++i) {
    cdata->_visgroups[info->_visgroups[i]].remove(info);
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

  if (bounds->is_infinite()) {
    return;
  }

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
}

/**
 *
 */
DynamicVisNode::CData::
CData() :
  _enabled(true)
{
}

/**
 *
 */
DynamicVisNode::CData::
CData(const CData &copy) :
  _visgroups(copy._visgroups),
  _enabled(copy._enabled)
{
}

/**
 *
 */
CycleData *DynamicVisNode::CData::
make_copy() const {
  return new CData(*this);
}
