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
#include "clockObject.h"

TypeHandle SceneVisibility::_type_handle;


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
get_box_sectors(const LPoint3 &mins, const LPoint3 &maxs, int *sectors, int &num_sectors) const {
  std::stack<int> node_stack;
  node_stack.push(0);

  while (!node_stack.empty()) {
    int node_index = node_stack.top();
    node_stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = &_sector_tree._nodes[node_index];

      if (maxs[node->axis] < node->dist) {
        // Completely behind the plane, traverse left.
        node_stack.push(node->left_child);

      } else if (mins[node->axis] >= node->dist) {
        // Completely in front of the plane, traverse right.
        node_stack.push(node->right_child);

      } else {
        // The box spans the plane, traverse both directions.
        node_stack.push(node->right_child);
        node_stack.push(node->left_child);
      }

    } else {
      // We reached a leaf node.
      const KDTree::Leaf *leaf = &_sector_tree._leaves[~node_index];
      if (leaf->value != -1) {
        bool already_set = false;
        for (int i = 0; i < num_sectors; ++i) {
          if (sectors[i] == leaf->value) {
            already_set = true;
            break;
          }
        }
        if (!already_set) {
          sectors[num_sectors++] = leaf->value;
          if (num_sectors >= 128) {
            return;
          }
        }
      }
    }
  }
}

/**
 * Fills the given BitArray with the set of sectors that the given sphere
 * overlaps with.
 */
void SceneVisibility::
get_sphere_sectors(const LPoint3 &center, PN_stdfloat radius, int *sectors, int &num_sectors) const {
  std::stack<int> node_stack;
  node_stack.push(0);

  while (!node_stack.empty()) {
    int node_index = node_stack.top();
    node_stack.pop();

    if (node_index >= 0) {
      const KDTree::Node *node = &_sector_tree._nodes[node_index];

      if ((center[node->axis] + radius) < node->dist) {
        // Completely behind the plane, traverse left.
        node_stack.push(node->left_child);

      } else if ((center[node->axis] - radius) >= node->dist) {
        // Completely in front of the plane, traverse right.
        node_stack.push(node->right_child);

      } else {
        // The sphere spans the plane, traverse both directions.
        node_stack.push(node->right_child);
        node_stack.push(node->left_child);
      }

    } else {
      // We reached a leaf node.
      const KDTree::Leaf *leaf = &_sector_tree._leaves[~node_index];
      if (leaf->value != -1) {
        bool already_set = false;
        for (int i = 0; i < num_sectors; ++i) {
          if (sectors[i] == leaf->value) {
            already_set = true;
            break;
          }
        }
        if (!already_set) {
          sectors[num_sectors++] = leaf->value;
          if (num_sectors >= 128) {
            return;
          }
        }
      }
    }
  }
}
