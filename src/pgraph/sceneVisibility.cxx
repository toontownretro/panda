/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneVisibility.cxx
 * @author brian
 * @date 2021-11-17
 */

#include "sceneVisibility.h"
#include <stack>
#include "nodePath.h"
#include "boundingBox.h"
#include "luse.h"
#include "boundingSphere.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"
#include "config_pgraph.h"
#include "pStatCollector.h"
#include "pStatTimer.h"

TypeHandle SceneVisibility::_type_handle;

static LightMutex vis_cache_lock("vis_cache");

static PStatCollector vis_test_collector("Cull:SceneVisTest");
static PStatCollector vis_cache_lookup("Cull:SceneVisTest:CacheLookup");
static PStatCollector vis_compare_transforms("Cull:SceneVisTest:CompareTransforms");

/**
 *
 */
SceneVisibility::
SceneVisibility() {
}

struct TravInfo {
  int node_index;
  int depth;
};

/**
 * Returns true if the given box is within any sectors of the given PVS,
 * false otherwise.
 *
 * Starts the traversal from the indicated node index, and locates the lowest
 * node in the tree that completely contains the box.  The returned lowest
 * node can be used as the head node for other boxes contained within this
 * box.
 */
bool SceneVisibility::
is_box_in_pvs(const LPoint3 &mins, const LPoint3 &maxs, const BitArray &pvs, int &lowest_node, int head_node) const {
  std::stack<TravInfo> node_stack;
  node_stack.push({ head_node, 0 });
  lowest_node = head_node;
  int lowest_depth = 0;

  while (!node_stack.empty()) {
    const TravInfo &info = node_stack.top();
    int node_index = info.node_index;
    int depth = info.depth;
    node_stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = &_sector_tree._nodes[node_index];

      if (maxs[node->axis] < node->dist) {
        // Completely behind the plane, traverse left.
        node_stack.push({ node->left_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->left_child;
        //}

      } else if (mins[node->axis] >= node->dist) {
        // Completely in front of the plane, traverse right.
        node_stack.push({ node->right_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->right_child;
       // }

      } else {
        // The box spans the plane, traverse both directions.
        node_stack.push({ node->right_child, depth + 1 });
        node_stack.push({ node->left_child, depth + 1 });
      }

    } else {
      // We reached a leaf node.
      const KDTree::Leaf *leaf = &_sector_tree._leaves[~node_index];
      if (leaf->value != -1 && pvs.get_bit(leaf->value)) {
        return true;
      }
    }
  }

  return false;
}

/**
 * Returns true if the given sphere is within any sectors of the given PVS,
 * false otherwise.
 *
 * Starts the traversal from the indicated node index, and locates the lowest
 * node in the tree that completely contains the sphere.  The returned lowest
 * node can be used as the head node for other spheres contained within this
 * sphere.
 */
bool SceneVisibility::
is_sphere_in_pvs(const LPoint3 &center, PN_stdfloat radius, const BitArray &pvs, int &lowest_node, int head_node) const {
  std::stack<TravInfo> node_stack;
  node_stack.push({ head_node, 0 });
  lowest_node = head_node;
  int lowest_depth = 0;

  while (!node_stack.empty()) {
    const TravInfo &info = node_stack.top();
    int node_index = info.node_index;
    int depth = info.depth;
    node_stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = &_sector_tree._nodes[node_index];

      if ((center[node->axis] + radius) < node->dist) {
        // Completely behind the plane, traverse left.
        node_stack.push({ node->left_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->left_child;
        //}

      } else if ((center[node->axis] - radius) >= node->dist) {
        // Completely in front of the plane, traverse right.
        node_stack.push({ node->right_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->right_child;
        //}

      } else {
        // The sphere spans the plane, traverse both directions.
        node_stack.push({ node->right_child, depth + 1 });
        node_stack.push({ node->left_child, depth + 1 });
      }

    } else {
      // We reached a leaf node.
      const KDTree::Leaf *leaf = &_sector_tree._leaves[~node_index];
      if (leaf->value != -1 && pvs.get_bit(leaf->value)) {
        return true;
      }
    }
  }

  return false;
}

/**
 * Fills the given BitArray with the set of sectors that the given box
 * overlaps with.
 */
void SceneVisibility::
get_box_sectors(const LPoint3 &mins, const LPoint3 &maxs, BitArray &sectors, int &lowest_node, int head_node) const {
  std::stack<TravInfo> node_stack;
  node_stack.push({ head_node, 0 });
  lowest_node = head_node;
  int lowest_depth = 0;

  while (!node_stack.empty()) {
    const TravInfo &info = node_stack.top();
    int node_index = info.node_index;
    int depth = info.depth;
    node_stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = &_sector_tree._nodes[node_index];

      if (maxs[node->axis] < node->dist) {
        // Completely behind the plane, traverse left.
        node_stack.push({ node->left_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->left_child;
        //}

      } else if (mins[node->axis] >= node->dist) {
        // Completely in front of the plane, traverse right.
        node_stack.push({ node->right_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->right_child;
       // }

      } else {
        // The box spans the plane, traverse both directions.
        node_stack.push({ node->right_child, depth + 1 });
        node_stack.push({ node->left_child, depth + 1 });
      }

    } else {
      // We reached a leaf node.
      const KDTree::Leaf *leaf = &_sector_tree._leaves[~node_index];
      if (leaf->value != -1 && !sectors.get_bit(leaf->value)) {
        sectors.set_bit(leaf->value);
      }
    }
  }
}

/**
 * Fills the given BitArray with the set of sectors that the given sphere
 * overlaps with.
 */
void SceneVisibility::
get_sphere_sectors(const LPoint3 &center, PN_stdfloat radius, BitArray &sectors, int &lowest_node, int head_node) const {
  std::stack<TravInfo> node_stack;
  node_stack.push({ head_node, 0 });
  lowest_node = head_node;
  int lowest_depth = 0;

  while (!node_stack.empty()) {
    const TravInfo &info = node_stack.top();
    int node_index = info.node_index;
    int depth = info.depth;
    node_stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = &_sector_tree._nodes[node_index];

      if ((center[node->axis] + radius) < node->dist) {
        // Completely behind the plane, traverse left.
        node_stack.push({ node->left_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->left_child;
        //}

      } else if ((center[node->axis] - radius) >= node->dist) {
        // Completely in front of the plane, traverse right.
        node_stack.push({ node->right_child, depth + 1 });

        //if (depth + 1 > lowest_depth) {
        //  lowest_depth = depth + 1;
        //  lowest_node = node->right_child;
        //}

      } else {
        // The sphere spans the plane, traverse both directions.
        node_stack.push({ node->right_child, depth + 1 });
        node_stack.push({ node->left_child, depth + 1 });
      }

    } else {
      // We reached a leaf node.
      const KDTree::Leaf *leaf = &_sector_tree._leaves[~node_index];
      if (leaf->value != -1 && !sectors.get_bit(leaf->value)) {
        sectors.set_bit(leaf->value);
      }
    }
  }
}

/**
 * Returns BoundingVolume::IntersectionFlags relating to the node's location
 * in the given PVS BitArray.
 *
 * IF_all - The node and all descendants are completely contained within the
 *          given PVS.
 * IF_some - The node is partially in the PVS and partially in vis sectors that
 *           aren't in the PVS.
 * IF_no_intersection - The node is not in the PVS at all.
 *
 * The vis sectors that a node is in are cached and only recomputed if the
 * net transform or bounding volume of the node changed since the last it was
 * asked.
 *
 * This version is intended to be called by higher level code, while the second
 * version (which this one calls into) is called by the more time sensitive
 * CullTraverser.
 */
int SceneVisibility::
is_node_in_pvs(const NodePath &node, const BitArray &pvs, const BitArray &inv_pvs) {
  int lowest_kd_node;
  CPT(TransformState) parent_net;
  if (!node.has_parent()) {
    parent_net = TransformState::make_identity();

  } else {
    parent_net = node.get_parent().get_net_transform();
  }

  return is_node_in_pvs(parent_net, node.get_bounds()->as_geometric_bounding_volume(),
                        node.node(), pvs, inv_pvs, lowest_kd_node, 0);
}

/**
 * Returns BoundingVolume::IntersectionFlags relating to the node's location
 * in the given PVS BitArray.
 *
 * IF_all - The node and all descendants are completely contained within the
 *          given PVS.
 * IF_some - The node is partially in the PVS and partially in vis sectors that
 *           aren't in the PVS.
 * IF_no_intersection - The node is not in the PVS at all.
 *
 * The vis sectors that a node is in are cached and only recomputed if the
 * net transform or bounding volume of the node changed since the last it was
 * asked.
 */
int SceneVisibility::
is_node_in_pvs(const TransformState *parent_net_transform, const GeometricBoundingVolume *bounds,
               PandaNode *node, const BitArray &pvs, const BitArray &inv_pvs,
               int &lowest_kd_node, int head_node) {
  PStatTimer timer(vis_test_collector);

  NodeVisData *vis_data = get_node_vis(node);

  // If the transform cache is in use, we can just compare the transforms by
  // pointer, as identical transforms are guaranteed to have the same pointer.
  // Otherwise, we need to compare the actual transforms.
  static bool using_transform_cache = transform_cache;

  bool transform_changed;
  vis_compare_transforms.start();
  if (using_transform_cache) {
    transform_changed = vis_data->parent_net_transform != parent_net_transform;

  } else if (vis_data->parent_net_transform != nullptr) {
    transform_changed = *vis_data->parent_net_transform != *parent_net_transform;

  } else {
    // It's fresh.
    transform_changed = true;
  }
  vis_compare_transforms.stop();

  if (transform_changed ||
      vis_data->node_bounds != bounds) {
    // The vis cache for the node is out of date.  Recompute it.

    vis_data->vis_sectors.clear();
    vis_data->parent_net_transform = parent_net_transform;
    vis_data->node_bounds = bounds;

    if (bounds->is_infinite()) {
      vis_data->vis_sectors = BitArray::all_on();
      vis_data->vis_head_node = head_node;

    } else if (bounds->is_exact_type(BoundingBox::get_class_type())) {
      const BoundingBox *bbox = (const BoundingBox *)bounds;

      LPoint3 mins = bbox->get_minq();
      LPoint3 maxs = bbox->get_maxq();

      if (!parent_net_transform->is_identity()) {
        // The net transform is non-identity.  We need to transform the
        // box into world coordinates for the K-D tree query.
        const LMatrix4 &mat = parent_net_transform->get_mat();

        LPoint3 x = bbox->get_point(0) * mat;
        LPoint3 n = x;
        for (int i = 1; i < 8; ++i) {
          LPoint3 p = bbox->get_point(i) * mat;
          n.set(std::min(n[0], p[0]), std::min(n[1], p[1]), std::min(n[2], p[2]));
          x.set(std::max(x[0], p[0]), std::max(x[1], p[1]), std::max(x[2], p[2]));
        }
        maxs = x;
        mins = n;
      }

      get_box_sectors(mins, maxs, vis_data->vis_sectors, vis_data->vis_head_node, head_node);

    } else if (bounds->is_exact_type(BoundingSphere::get_class_type())) {
      const BoundingSphere *bsphere = (const BoundingSphere *)bounds;

      PN_stdfloat radius = bsphere->get_radius();
      LPoint3 center = bsphere->get_center();

      if (!parent_net_transform->is_identity()) {
        // The net transform is non-identity.  We need to transform the
        // sphere into world coordinates for the K-D tree query.
        const LMatrix4 &mat = parent_net_transform->get_mat();

        // First, determine the longest axis of the matrix, in case it contains a
        // non-uniform scale.

        LVecBase3 x, y, z;
        mat.get_row3(x, 0);
        mat.get_row3(y, 1);
        mat.get_row3(z, 2);

        PN_stdfloat xd = dot(x, x);
        PN_stdfloat yd = dot(y, y);
        PN_stdfloat zd = dot(z, z);

        PN_stdfloat scale = std::max(xd, yd);
        scale = std::max(scale, zd);
        scale = csqrt(scale);

        // Transform the radius
        radius *= scale;

        // Transform the center
        center = center * mat;
      }

      get_sphere_sectors(center, radius, vis_data->vis_sectors, vis_data->vis_head_node, head_node);

    } else {
      // If for some reason the node has a bounding volume that isn't a box
      // or sphere, forget it and just say it's in the PVS.  I want to avoid
      // the overhead of the BoundingVolume interface for this check and it
      // would be a pain to implement a K-D tree query for each bounding volume
      // type.
      vis_data->vis_sectors = BitArray::all_on();
      vis_data->vis_head_node = head_node;
    }
  }

  lowest_kd_node = vis_data->vis_head_node;

  if (!pvs.has_bits_in_common(vis_data->vis_sectors)) {
    return BoundingVolume::IF_no_intersection;

  } else if (inv_pvs.has_bits_in_common(vis_data->vis_sectors)) {
    return BoundingVolume::IF_some;

  } else {
    return BoundingVolume::IF_all;
  }
}

/**
 * Returns the visibility cache for the indicated PandaNode.
 * If the node is not already in the visibility cache, creates an entry
 * in the cache for the node and returns the new entry.
 *
 * Don't keep the pointer to the vis data around as it may become
 * invalidated if the cache is modified.
 */
SceneVisibility::NodeVisData *SceneVisibility::
get_node_vis(PandaNode *node) {
  //LightMutexHolder holder(vis_cache_lock);

  PStatTimer timer(vis_cache_lookup);

  auto it = _node_vis_cache.find(node);
  if (it != _node_vis_cache.end()) {
    return (*it).second;
  }

  // Node not in cache.  Create new entry.
  PT(NodeVisData) data = new NodeVisData;
  data->node_bounds = nullptr;
  data->parent_net_transform = nullptr;
  data->vis_head_node = 0;

  _node_vis_cache.insert({ node, data });

  WeakReferenceList *weak_list = node->get_weak_list();
  weak_list->add_callback(this, node);

  return data;
}

/**
 *
 */
void SceneVisibility::
wp_callback(void *data) {
  PandaNode *node = (PandaNode *)data;
  auto it = _node_vis_cache.find(node);
  if (it != _node_vis_cache.end()) {
    _node_vis_cache.erase(it);
  }
}
