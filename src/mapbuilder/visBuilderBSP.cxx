/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visBuilderBSP.cxx
 * @author brian
 * @date 2021-12-21
 */

#include "visBuilderBSP.h"
#include "mathutil_misc.h"
#include "mapBuilder.h"
#include "keyValues.h"
#include "threadManager.h"

#include <stack>

#ifndef CheckBit
#define CheckBit( bitstring, bitNumber )	( (bitstring)[ ((bitNumber) >> 3) ] & ( 1 << ( (bitNumber) & 7 ) ) )
#define SetBit( bitstring, bitNumber )	( (bitstring)[ ((bitNumber) >> 3) ] |= ( 1 << ( (bitNumber) & 7 ) ) )
#define ClearBit( bitstring, bitNumber )	( (bitstring)[ ((bitNumber) >> 3)
#endif

static constexpr PN_stdfloat clip_epsilon = 0.1f;

static int
count_bits(const unsigned char *bits, size_t num_bits) {
  int c = 0;
  for (size_t i = 0; i < num_bits; i++) {
    if (CheckBit(bits, i)) {
      c++;
    }
  }
  return c;
}

/**
 * Returns the general axis of the indicated plane normal.
 */
static planetypes
get_plane_type(const LPlane &plane) {
  vec3_t normal;
  normal[0] = plane[0];
  normal[1] = plane[1];
  normal[2] = plane[2];
  return PlaneTypeForNormal(normal);
}

/**
 * Clips the solid by the given plane.  Returns the two clipped solids in
 * front of and behind the plane.
 */
void BSPSolid::
clip(const LPlane &plane, PT(BSPSolid) &front, PT(BSPSolid) &back) const {
  front = nullptr;
  back = nullptr;

  pvector<LPlane> front_planes, back_planes;

  for (BSPFace *face : _faces) {
    PlaneSide s = face->_winding.get_plane_side(plane);
    if (s == PS_front) {
      front_planes.push_back(face->_winding.get_plane());

    } else if (s == PS_back) {
      back_planes.push_back(face->_winding.get_plane());

    } else if (s == PS_cross) {
      front_planes.push_back(face->_winding.get_plane());
      back_planes.push_back(face->_winding.get_plane());
    }
  }

  // Close off the clipped solids with the clipping plane.
  if (front_planes.size() >= 2u) {
    front_planes.push_back(-plane);

    front = new BSPSolid;
    front->_opaque = _opaque;

    // Create the solid faces
    for (size_t i = 0; i < front_planes.size(); ++i) {
      Winding w(front_planes[i]);
      for (size_t j = 0; j < front_planes.size(); ++j) {
        if (i == j) {
          continue;
        }
        w = w.chop(-front_planes[j]);
      }
      PT(BSPFace) f = new BSPFace;
      f->_winding = w;
      f->_priority = 0;
      f->_hint = false;
      f->_contents = 0;
      front->_faces.push_back(f);
    }
  }

  if (back_planes.size() >= 2u) {
    back_planes.push_back(plane);

    back = new BSPSolid;
    back->_opaque = _opaque;

    // Create the solid faces
    for (size_t i = 0; i < back_planes.size(); ++i) {
      Winding w(back_planes[i]);
      for (size_t j = 0; j < back_planes.size(); ++j) {
        if (i == j) {
          continue;
        }
        w = w.chop(-back_planes[j]);
      }
      PT(BSPFace) f = new BSPFace;
      f->_winding = w;
      f->_priority = 0;
      f->_hint = false;
      f->_contents = 0;
      back->_faces.push_back(f);
    }
  }
}

struct MGFilterStack {
  BSPNode *node;
  Winding winding;
};

/**
 *
 */
bool VisBuilderBSP::
bake() {
  // Start by constructing the BSP tree from the occluder polygons.
  if (!build_bsp_tree()) {
    return false;
  }

  // Each leaf node of the BSP tree is a visibility cell/volume.

  // Find connections between leaves (portals).
  if (!build_portals()) {
    return false;
  }

  filter_structural_solids_into_tree();

  if (flood_entities()) {
    r_fill_outside(_tree_root);
    mark_visible_sides();

    // Now remove invisible faces.
    for (auto it = _input_faces.begin(); it != _input_faces.end();) {
      BSPFace *face = *it;
      if (!face->_visible || face->_winding.is_empty()) {
        it = _input_faces.erase(it);
      } else {
        ++it;
      }
    }

    // Rebuild the BSP tree using only the visible face list.
    build_bsp_tree();
    build_portals();
    filter_structural_solids_into_tree();

    // Remove portals that lead to/from solid leaves.
    r_remove_opaque_portals(_tree_root);

    // Build portal representations for vis.
    r_build_portal_list(_tree_root);

    // Assign mesh groups to BSP leaves.
    for (size_t i = 0; i < _builder->_mesh_groups.size(); ++i) {
      MapGeomGroup *group = &_builder->_mesh_groups[i];

      for (size_t j = 0; j < group->geoms.size(); ++j) {
        MapPoly *poly = (MapPoly *)group->geoms[j];
        std::stack<MGFilterStack> node_stack;
        node_stack.push({ _tree_root, poly->_winding });
        while (!node_stack.empty()) {
          MGFilterStack data = node_stack.top();
          node_stack.pop();
          BSPNode *node = data.node;
          if (!node->is_leaf()) {
            // Clip face into children.
            PlaneSide s = data.winding.get_plane_side(node->_plane);
            if (s == PS_on) {
              // Winding lies on node plane.
              // Compare normals to determine direction to traverse.
              LPlane plane = data.winding.get_plane();
              if (plane.get_normal().dot(node->_plane.get_normal()) >= 0.999f) {
                // Winding is facing node plane direction, traverse forward.
                node_stack.push({ node->_children[FRONT_CHILD], data.winding });
              } else {
                // Facing away, traverse behind.
                node_stack.push({ node->_children[BACK_CHILD], data.winding });
              }

            } else {
              Winding front, back;
              data.winding.clip_epsilon(node->_plane, 0.001, front, back);
              node_stack.push({ node->_children[FRONT_CHILD], front });
              node_stack.push({ node->_children[BACK_CHILD], back });
            }

          } else {
            if (!data.winding.is_empty() && node->_leaf_id != -1) {
              // Valid winding left over in this leaf.  Assign the group to
              // this leaf.
              group->clusters.set_bit(node->_leaf_id);
            }
          }
        }
      }
    }

    mapbuilder_cat.info()
      << _portal_list.size() << " numportals\n";

    _portal_bytes = ((_portal_list.size() + 63) & ~63) >> 3;
    _portal_longs = _portal_bytes / sizeof(long);

    ThreadManager::run_threads_on_individual(
      "BasePortalVis", _portal_list.size(), false,
      std::bind(&VisBuilderBSP::base_portal_vis, this, std::placeholders::_1));

    sort_portals();

    ThreadManager::run_threads_on_individual(
      "PortalFlow", _portal_list.size(), false,
      std::bind(&VisBuilderBSP::portal_flow, this, std::placeholders::_1));

    ThreadManager::run_threads_on_individual(
      "FinalLeafPVS", _empty_leaf_list.size(), false,
      std::bind(&VisBuilderBSP::final_leaf_pvs, this, std::placeholders::_1));

    // Store PVS data on the output map.
    for (size_t i = 0; i < _empty_leaf_list.size(); ++i) {
      AreaClusterPVS pvs;

      for (int leaf_id : _empty_leaf_list[i]->_pvs) {
        pvs.add_visible_cluster(leaf_id);
      }

      // Assign mesh groups to the cluster.
      int mesh_group_index = 0;
      for (auto it = _builder->_mesh_groups.begin(); it != _builder->_mesh_groups.end(); ++it) {
        if (it->clusters.get_bit(_empty_leaf_list[i]->_leaf_id)) {
          // Mesh group resides in this area cluster.
          pvs.set_mesh_group(mesh_group_index);
        }
        mesh_group_index++;
      }

      // Store the AABB of the leaf for debug visualization in the show.
      pvs._box_bounds.push_back(_empty_leaf_list[i]->_mins);
      pvs._box_bounds.push_back(_empty_leaf_list[i]->_maxs);

      _builder->_out_data->add_cluster_pvs(std::move(pvs));
    }

  } else {
    mapbuilder_cat.warning()
      << "****** leaked ******\n";
  }

  if (!build_output_tree()) {
    return false;
  }

  return true;
}

/**
 * Builds the final BSP tree structure for runtime use.
 */
bool VisBuilderBSP::
build_output_tree() {
  _output_tree = new BSPTree;

  // Start with the flat leaf list.
  for (BSPNode *leaf : _leaf_list) {
    BSPTree::Leaf oleaf;
    oleaf.solid = leaf->_opaque;
    if (!leaf->_opaque) {
      oleaf.value = leaf->_leaf_id;
    }
    _output_tree->_leaves.push_back(std::move(oleaf));
  }
  _output_tree->_leaf_parents.resize(_output_tree->_leaves.size());

  r_build_output_tree(_tree_root, -1);

  _builder->_out_data->set_area_cluster_tree(_output_tree);

  return true;
}

/**
 *
 */
int VisBuilderBSP::
r_build_output_tree(const BSPNode *node, int parent) {
  if (!node->is_leaf()) {
    int node_idx = (int)_output_tree->_nodes.size();
    _output_tree->_nodes.push_back(BSPTree::Node());
    _output_tree->_node_parents.push_back(parent);
    _output_tree->_nodes[node_idx].plane = node->_plane;
    if (node->_children[0] != nullptr) {
      _output_tree->_nodes[node_idx].children[0] = r_build_output_tree(node->_children[0], node_idx);
    }
    if (node->_children[1] != nullptr) {
      _output_tree->_nodes[node_idx].children[1] = r_build_output_tree(node->_children[1], node_idx);
    }
    return node_idx;

  } else {
    _output_tree->_leaf_parents[node->_leaf_index] = parent;
    return ~((int)node->_leaf_index);
  }
}

/**
 * Constructs a BSP tree from the input occluder and hint polygons.  Leaf
 * nodes are treated as convex visibility cells.
 */
bool VisBuilderBSP::
build_bsp_tree() {
  _outside_node = new BSPNode;
  // First start with a root node that encloses all of the input polygons,
  // and pick the first place to split (and recurse).
  _tree_root = new BSPNode;
  // Calc bounds from all faces.
  _tree_root->_mins = LPoint3(9999999);
  _tree_root->_maxs = LPoint3(-9999999);
  for (size_t i = 0; i < _input_faces.size(); i++) {
    const BSPFace *face = _input_faces[i];
    LPoint3 face_mins, face_maxs;
    face->_winding.get_bounds(face_mins, face_maxs);
    for (int j = 0; j < 3; j++) {
      _tree_root->_mins[j] = std::min(_tree_root->_mins[j], face_mins[j]);
    }
    for (int j = 0; j < 3; j++) {
      _tree_root->_maxs[j] = std::max(_tree_root->_maxs[j], face_maxs[j]);
    }
  }
  make_subtree(_tree_root, _input_faces);
  return true;
}

/**
 *
 */
bool VisBuilderBSP::
build_portals() {
  make_headnode_portals();
  r_make_tree_portals(_tree_root);
  return true;
}

/**
 * Places a portal
 */
void VisBuilderBSP::
make_headnode_portals() {
  BSPNode *node = _tree_root;
  BSPNode *outside_node = _outside_node;

  PT(BSPPortal) portals[6];
  LPlane bplanes[6];
  LPoint3 bounds[2];

  static constexpr int side_space = 8;

  // Pad with some space so there will never be null volume leaves.
  for (int i = 0; i < 3; i++) {
    bounds[0][i] = node->_mins[i] - side_space;
    bounds[1][i] = node->_maxs[i] + side_space;
    if (bounds[0][i] >= bounds[1][i]) {
      nassert_raise("Backwards tree volume");
      abort();
    }
  }

  outside_node->_opaque = false;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      int n = j * 3 + i;

      PT(BSPPortal) p = new BSPPortal;
      portals[n] = p;

      LPlane &pl = bplanes[n];
      pl = LPlane(0, 0, 0, 0);
      if (j) {
        pl[i] = -1;
        pl[3] = bounds[j][i];
      } else {
        pl[i] = 1;
        pl[3] = -bounds[j][i];
      }
      p->_plane = pl;
      p->_winding = Winding(pl);
      add_portal_to_nodes(p, node, outside_node);
    }
  }

  // Clip the portal windings by all other box planes.
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      if (j == i) {
        continue;
      }

      Winding chopped_winding = portals[i]->_winding.chop(bplanes[j]);
      assert(!chopped_winding.is_empty());
      portals[i]->_winding = chopped_winding;
    }
  }
}

/**
 *
 */
void VisBuilderBSP::
r_make_tree_portals(BSPNode *node) {
  calc_node_portal_bounds(node);
  if (node->_mins[0] >= node->_maxs[0]) {
    mapbuilder_cat.error()
      << "WARNING: node without a volume\n";
    abort();
  }

  if (node->is_leaf()) {
    return;
  }

  make_node_portal(node);
  split_node_portals(node);

  r_make_tree_portals(node->_children[0]);
  r_make_tree_portals(node->_children[1]);
}

/**
 *
 */
void VisBuilderBSP::
make_node_portal(BSPNode *node) {

  Winding w = get_node_winding(node);

  // Clip the portal by all other portals in the node.
  for (BSPPortal *p : node->_portals) {
    LPlane plane = p->_plane;
    if (p->_nodes[1] == node) {
      plane = -plane;
    }

    w = w.chop(plane);
    if (w.is_empty()) {
      continue;
    }
  }

  if (w.is_empty() || w.is_tiny()) {
    return;
  }

  PT(BSPPortal) new_portal = new BSPPortal;
  new_portal->_plane = node->_plane;
  new_portal->_on_node = node;
  new_portal->_winding = w;
  new_portal->_hint = node->_hint;
  add_portal_to_nodes(new_portal, node->_children[FRONT_CHILD], node->_children[BACK_CHILD]);
}

/**
 *
 */
void VisBuilderBSP::
split_node_portals(BSPNode *node) {
  LPlane plane = node->_plane;

  BSPNode *f = node->_children[FRONT_CHILD];
  BSPNode *b = node->_children[BACK_CHILD];

  pvector<PT(BSPPortal)> node_portals = node->_portals;
  for (BSPPortal *p : node_portals) {
    int side;
    if (p->_nodes[0] == node) {
      side = 0;
    } else if (p->_nodes[1] == node) {
      side = 1;
    } else {
      mapbuilder_cat.error()
        << "SplitNodePortals: mislinked portal\n";
      abort();
    }

    BSPNode *other_node = p->_nodes[!side];
    remove_portal_from_node(p, p->_nodes[0]);
    remove_portal_from_node(p, p->_nodes[1]);

    // Cut the portal into two portals, one on each side of the cut plane.
    Winding front, back;
    p->_winding.clip_epsilon(plane, 0.001, front, back);

    if (front.is_tiny()) {
      front.clear();
    }
    if (back.is_tiny()) {
      back.clear();
    }

    if (front.is_empty() && back.is_empty()) {
      continue;
    }

    if (front.is_empty()) {
      if (side == 0) {
        add_portal_to_nodes(p, b, other_node);
      } else {
        add_portal_to_nodes(p, other_node, b);
      }
      continue;
    }

    if (back.is_empty()) {
      if (side == 0) {
        add_portal_to_nodes(p, f, other_node);
      } else {
        add_portal_to_nodes(p, other_node, f);
      }
      continue;
    }

    // The winding is split.

    PT(BSPPortal) new_portal = new BSPPortal(*p);
    new_portal->_winding = back;
    p->_winding = front;

    if (side == 0) {
      add_portal_to_nodes(p, f, other_node);
      add_portal_to_nodes(new_portal, b, other_node);

    } else {
      add_portal_to_nodes(p, other_node, f);
      add_portal_to_nodes(new_portal, other_node, b);
    }
  }

  // Portals on this node have been moved to children.
  node->_portals.clear();
}

/**
 * Returns a winding along the node of the plane, clipped by all of the node's
 * parents.
 */
Winding VisBuilderBSP::
get_node_winding(BSPNode *node) {
  Winding w(node->_plane);

  // Clip by all parents.
  for (BSPNode *n = node->_parent; n != nullptr && !w.is_empty(); ) {
    LPlane plane = n->_plane;

    if (n->_children[FRONT_CHILD] == node) {
      // Take front.
      w = w.chop(plane);

    } else {
      // Take back.
      w = w.chop(-plane);
    }

    node = n;
    n = n->_parent;
  }

  return w;
}

/**
 *
 */
bool VisBuilderBSP::
calc_pvs() {
  return true;
}

/**
 * Partitions the polygons at the node into two sides, picks the best split
 * from all polygon planes.
 */
void VisBuilderBSP::
make_subtree(BSPNode *node, const BSPFaces &faces) {
  if (faces.empty()) {
    // If we have no more polygons, this is a leaf node.
    return;
  }

  // Otherwise we need to partition our polygons along an arbitrary
  // hyperplane.  Pick the best polygon plane to split along.
  int split_idx = pick_best_split(faces);
  if (split_idx < 0) {
    // I don't think this should be possible since we checked if we have
    // no polygons above.
    return;
  }

  LPlane split_plane = faces[split_idx]->_winding.get_plane();

  node->_plane = split_plane;
  node->_hint = _hint_split;

  BSPFaces front_polys, back_polys;
  front_polys.reserve(faces.size());
  back_polys.reserve(faces.size());

  // We picked a split, now classify all our polygons against that plane.
  // Polygons on the split plane are kept at the node and not partitioned
  // further.
  for (size_t i = 0; i < faces.size(); i++) {
    PlaneSide side = faces[i]->_winding.get_plane_side(split_plane);
    if (side == PS_cross) {
      // Face crosses the chosen split plane.  Clip the polygon to the plane,
      // add the new polygons to the correct sides.

      Winding front, back;
      faces[i]->_winding.clip_epsilon(split_plane, clip_epsilon * 2, front, back);

      // We should definitely have a polygon on both sides.
      //assert(!front.is_empty());
      //assert(!back.is_empty());

      if (!front.is_empty()) {
        PT(BSPFace) front_face = new BSPFace;
        front_face->_winding = front;
        front_face->_hint = faces[i]->_hint;
        front_polys.push_back(front_face);
      }
      if (!back.is_empty()) {
        PT(BSPFace) back_face = new BSPFace;
        back_face->_winding = back;
        back_face->_hint = faces[i]->_hint;
        back_polys.push_back(back_face);
      }

    } else if (side == PS_front) {
      front_polys.push_back(faces[i]);

    } else if (side == PS_back) {
      back_polys.push_back(faces[i]);
    }
  }

  // Now make our children subtrees using the partitioned polygons.
  node->_children[BACK_CHILD] = new BSPNode;
  node->_children[BACK_CHILD]->_parent = node;
  node->_children[BACK_CHILD]->_mins = node->_mins;
  node->_children[BACK_CHILD]->_maxs = node->_maxs;

  node->_children[FRONT_CHILD] = new BSPNode;
  node->_children[FRONT_CHILD]->_parent = node;
  node->_children[FRONT_CHILD]->_mins = node->_mins;
  node->_children[FRONT_CHILD]->_maxs = node->_maxs;

  // Clip child bounds to node split plane.
  for (int i = 0; i < 3; i++) {
    if (split_plane.get_normal()[i] == 1) {
      node->_children[BACK_CHILD]->_maxs[i] = split_plane.get_distance();
      node->_children[FRONT_CHILD]->_mins[i] = split_plane.get_distance();
      break;
    }
  }

  make_subtree(node->_children[BACK_CHILD], back_polys);
  make_subtree(node->_children[FRONT_CHILD], front_polys);
}

/**
 *
 */
int VisBuilderBSP::
pick_best_split(const BSPFaces &faces) {
  _hint_split = false;

  int best_value = -99999999;
  int best_split = -1;

  for (BSPFace *face : faces) {
    face->_checked = false;
  }

  for (size_t i = 0; i < faces.size(); i++) {
    const BSPFace *face = faces[i];

    if (face->_checked) {
      continue;
    }

    // Compute how good the split would be if we split along this polygon's
    // plane.  The polygon plane with the highest split score will be used.

    LPlane plane = face->_winding.get_plane();
    int splits = 0;
    int facing = 0;
    int front = 0;
    int back = 0;
    for (size_t j = 0; j < faces.size(); j++) {
      const Winding *check_winding = &faces[j]->_winding;
      LPlane check_plane = check_winding->get_plane();

      if (check_plane.almost_equal(plane)) {
        // We don't need to test this plane again.
        facing++;
        faces[j]->_checked = true;
        continue;
      }

      PlaneSide side = check_winding->get_plane_side(plane);
      switch (side) {
      case PS_cross:
        splits++;
        break;
      case PS_front:
        front++;
        break;
      case PS_back:
        back++;
        break;
      default:
        break;
      }
    }

    int value = 5 * facing - 5 * splits - abs(front - back);

    // Axial splits are better.
    if (get_plane_type(plane) < plane_anyx) {
      value += 5;
    }

    // Prioritize hints better.
    value += face->_priority;

    if (value > best_value) {
      best_value = value;
      best_split = (int)i;
    }
  }

  if (best_value == -99999999) {
    return -1;
  }

  if (faces[best_split]->_hint) {
    _hint_split = true;
  }

  return best_split;
}

/**
 * Adds a portal that connects the given two nodes.
 */
void VisBuilderBSP::
add_portal_to_nodes(BSPPortal *portal, BSPNode *front, BSPNode *back) {
  portal->_nodes[0] = front;
  front->_portals.push_back(portal);

  portal->_nodes[1] = back;
  back->_portals.push_back(portal);
}

/**
 * Removes a portal from the given node.
 */
void VisBuilderBSP::
remove_portal_from_node(BSPPortal *portal, BSPNode *node) {
  if (portal->_nodes[0] == node) {
    portal->_nodes[0] = nullptr;

  } else if (portal->_nodes[1] == node) {
    portal->_nodes[1] = nullptr;
  }

  auto it = std::find(node->_portals.begin(), node->_portals.end(), portal);
  if (it != node->_portals.end()) {
    node->_portals.erase(it);
  }
}

/**
 * Calculates the bounding box of the portals of the given node.
 */
void VisBuilderBSP::
calc_node_portal_bounds(BSPNode *node) {
  node->_mins.set(9999999, 9999999, 9999999);
  node->_maxs.set(-9999999, -9999999, -9999999);
  for (BSPPortal *portal : node->_portals) {
    for (int i = 0; i < portal->_winding.get_num_points(); i++) {
      LPoint3 point = portal->_winding.get_point(i);
      for (int j = 0; j < 3; j++) {
        node->_mins[j] = std::min(node->_mins[j], point[j]);
      }
      for (int j = 0; j < 3; j++) {
        node->_maxs[j] = std::max(node->_maxs[j], point[j]);
      }
    }
  }
}

/**
 *
 */
bool VisBuilderBSP::
place_occupant(BSPNode *node, const LPoint3 &origin) {
  while (!node->is_leaf()) {
    PN_stdfloat d = node->_plane.dist_to_plane(origin);
    if (d >= 0) {
      node = node->_children[FRONT_CHILD];
    } else {
      node = node->_children[BACK_CHILD];
    }
  }

  if (node->_opaque) {
    return false;
  }

  // Mark this node and all nodes reachable through portals from this
  // node as occupied.
  r_flood_portals(node);

  return true;
}

/**
 *
 */
void VisBuilderBSP::
r_flood_portals(BSPNode *node) {
  if (node->_occupied || node->_opaque) {
    return;
  }

  node->_occupied = true;

  // Flood outward through portals.
  for (BSPPortal *portal : node->_portals) {
    int s = (portal->_nodes[1] == node);
    r_flood_portals(portal->_nodes[!s]);
  }
}

/**
 *
 */
bool VisBuilderBSP::
flood_entities() {
  _outside_node->_occupied = false;

  bool inside = false;

  MapFile *src_map = _builder->_source_map;
  for (size_t i = 1; i < src_map->_entities.size(); ++i) {
    MapEntitySrc *ent = src_map->_entities[i];
    auto origin_it = ent->_properties.find("origin");
    if (origin_it == ent->_properties.end()) {
      continue;
    }
    LPoint3 origin = KeyValues::to_3f(origin_it->second);
    if (origin == LPoint3(0)) {
      continue;
    }

    // So objects on floor are okay.
    origin[2] += 1;

    // Find the leaf of the entity.
    if (place_occupant(_tree_root, origin)) {
      inside = true;
    }
  }

  if (!inside) {
    mapbuilder_cat.info()
      << "No entities in open -- no filling\n";

  } else if (_outside_node->_occupied) {
    mapbuilder_cat.info()
      << "Entity reached from outside -- no filling\n";
  }

  return inside && !_outside_node->_occupied;
}

/**
 * Fills away nodes that cannot be reached by any entity.  Removes portals to
 * and from unreachable nodes.
 */
void VisBuilderBSP::
r_fill_outside(BSPNode *node) {
  if (!node->is_leaf()) {
    r_fill_outside(node->_children[FRONT_CHILD]);
    r_fill_outside(node->_children[BACK_CHILD]);
    return;
  }

  if (!node->_occupied) {
    // No entity can reach this node, fill it away.
    node->_opaque = true;
  }
}

/**
 * Removes portals leading to/from opaque nodes.
 */
void VisBuilderBSP::
r_remove_opaque_portals(BSPNode *node) {
  if (!node->is_leaf()) {
    r_remove_opaque_portals(node->_children[FRONT_CHILD]);
    r_remove_opaque_portals(node->_children[BACK_CHILD]);
    return;
  }

  if (node->_opaque) {
    // The node is opaque/solid.  Any portals on this node are invalid and
    // should be removed.

    // Remove the portal to this node from the other nodes.
    for (BSPPortal *p : node->_portals) {
      int other_side = p->_nodes[0] == node;
      BSPNode *other_node = p->_nodes[other_side];

      auto it = std::find(other_node->_portals.begin(), other_node->_portals.end(), p);
      if (it != other_node->_portals.end()) {
        other_node->_portals.erase(it);
      }
    }

    // Remove portals from this node.
    node->_portals.clear();
  }
}

/**
 * Marks leaves that contain structural brushes/solids as opaque.
 * In other words, it determines which leaves are solid and which are empty.
 */
void VisBuilderBSP::
filter_structural_solids_into_tree() {
  for (BSPSolid *solid : _input_solids) {
    r_filter_structural_solid_into_tree(solid, _tree_root);
  }
}

/**
 *
 */
int VisBuilderBSP::
r_filter_structural_solid_into_tree(BSPSolid *solid, BSPNode *node) {
  if (solid == nullptr) {
    return 0;
  }

  // There is a left-over solid in this leaf.  Mark the leaf solid/opaque.
  if (node->is_leaf()) {
    // If solid has no hint or skipped sides, leaf is opaque.
    if (solid->_opaque) {
      node->_opaque = true;
    }
    return 1;
  }

  // Clip solid to node plane.
  PT(BSPSolid) front, back;
  solid->clip(node->_plane, front, back);

  int c = 0;
  if (front != nullptr) {
    c += r_filter_structural_solid_into_tree(front, node->_children[FRONT_CHILD]);
  }
  if (back != nullptr) {
    c += r_filter_structural_solid_into_tree(back, node->_children[BACK_CHILD]);
  }
  return c;
}

/**
 * Marks BSP faces visible from the interior of the BSP tree as visible.
 */
void VisBuilderBSP::
mark_visible_sides() {
  int num_visible_faces = 0;

  mapbuilder_cat.info()
    << _input_faces.size() << " total faces\n";

  for (BSPFace *face : _input_faces) {
    face->_visible = false;
    r_mark_visible_sides(face, face->_winding, _tree_root);
    if (face->_visible) {
      num_visible_faces++;
    }
  }

  mapbuilder_cat.info()
    << num_visible_faces << " visible faces\n";
}

/**
 *
 */
void VisBuilderBSP::
r_mark_visible_sides(BSPFace *face, Winding winding, BSPNode *node) {
  if (winding.is_empty()) {
    return;
  }

  if (node->is_leaf()) {
    if (!node->_opaque) {
      // Face reached an empty leaf.  It's visible from the interior.
      face->_visible = true;
    }
    return;
  }

  // Clip face into children.
  PlaneSide s = winding.get_plane_side(node->_plane);
  if (s == PS_on) {
    // Winding lies on node plane.
    // Compare normals to determine direction to traverse.
    LPlane plane = winding.get_plane();
    if (plane.get_normal().dot(node->_plane.get_normal()) >= 0.999f) {
      // Winding is facing node plane direction, traverse forward.
      r_mark_visible_sides(face, winding, node->_children[FRONT_CHILD]);
    } else {
      // Facing away, traverse behind.
      r_mark_visible_sides(face, winding, node->_children[BACK_CHILD]);
    }

  } else {
    Winding front, back;
    winding.clip_epsilon(node->_plane, 0.001, front, back);
    r_mark_visible_sides(face, front, node->_children[FRONT_CHILD]);
    r_mark_visible_sides(face, back, node->_children[BACK_CHILD]);
  }
}

/**
 * Collects all leaf portals into a single list for later PVS computation.
 */
void VisBuilderBSP::
r_build_portal_list(BSPNode *node) {
  if (node->is_leaf()) {
    for (BSPPortal *p : node->_portals) {
      int side = p->_nodes[1] == node;

      if (p->_nodes[0]->_opaque || p->_nodes[1]->_opaque) {
        continue;
      }

      PT(BSPVisPortal) vp = new BSPVisPortal;
      vp->_winding = p->_winding;
      vp->_hint = p->_hint;
      vp->_leaf = p->_nodes[!side];
      vp->_plane = -p->_plane;
      if (side) {
        // Back-side portal.
        vp->_winding.reverse();
        vp->_plane.flip();
      }
      vp->_origin = vp->_winding.get_center();
      vp->_id = _portal_list.size();
      _portal_list.push_back(vp);
      node->_vis_portals.push_back(vp);
    }

    node->_leaf_index = (int)_leaf_list.size();
    _leaf_list.push_back(node);
    if (!node->_opaque) {
      node->_leaf_id = (int)_empty_leaf_list.size();
      _empty_leaf_list.push_back(node);
    }

  } else {
    r_build_portal_list(node->_children[FRONT_CHILD]);
    r_build_portal_list(node->_children[BACK_CHILD]);
  }
}

/**
 *
 */
void VisBuilderBSP::
base_portal_vis(int i) {
  BSPVisPortal *p = _portal_list[i];

  p->calc_radius();

   // Allocate memory for bitwise vis solutions for this portal.
  p->_portal_front = new unsigned char[_portal_bytes];
  memset(p->_portal_front, 0, _portal_bytes);
  p->_portal_flood = new unsigned char[_portal_bytes];
  memset(p->_portal_flood, 0, _portal_bytes);
  p->_portal_vis = new unsigned char[_portal_bytes];
  memset(p->_portal_vis, 0, _portal_bytes);
  p->_num_might_see = 0;

  // Test the portal against all of the other portals in the map.
  for (size_t j = 0; j < _portal_list.size(); j++) {
    if (j == i) {
      // Don't test against itself.
      continue;
    }

    BSPVisPortal *tp = _portal_list[j];

    Winding *w = &(tp->_winding);
    // Classify the other portal against the plane of this portal.
    PlaneSide other_side = w->get_plane_side(p->_plane);
    if (other_side == PS_back ||
        other_side == PS_on) {
      // Other portal lies on or is completely behind this portal.  There's
      // no way we can see it.
      continue;
    }

    w = &(p->_winding);
    // Now classify myself against the plane of the other portal.
    PlaneSide my_side = w->get_plane_side(tp->_plane);
    if (my_side == PS_front) {
      // This portal is completely in front of the other portal.  There's
      // no way we can see it.
      continue;
    }

    // Add current portal to given portal's list of visible portals.
    SetBit(p->_portal_front, j);
  }

  simple_flood(p, p->_leaf);

  p->_num_might_see = count_bits(p->_portal_flood, _portal_list.size());
}

/**
 *
 */
void VisBuilderBSP::
simple_flood(BSPVisPortal *src_portal, BSPNode *node) {
  for (BSPVisPortal *p : node->_vis_portals) {
    int pnum = p->_id;
    if (!CheckBit(src_portal->_portal_front, pnum)) {
      continue;
    }
    if (CheckBit(src_portal->_portal_flood, pnum)) {
      continue;
    }
    SetBit(src_portal->_portal_flood, pnum);
    simple_flood(src_portal, p->_leaf);
  }
}

/**
 * Sorts the portals from the least complex, so the later ones can reuse the
 * earlier information.
 */
void VisBuilderBSP::
sort_portals() {
  _sorted_portals.insert(_sorted_portals.end(), _portal_list.begin(), _portal_list.end());
  std::sort(_sorted_portals.begin(), _sorted_portals.end(),
    [](const BSPVisPortal *a, const BSPVisPortal *b) -> bool {
      return a->_num_might_see < b->_num_might_see;
    });
}

/**
 *
 */
void VisBuilderBSP::
portal_flow(int i) {
  BSPVisPortal *p = _sorted_portals[i];
  AtomicAdjust::set(p->_status, BSPVisPortal::S_working);
  BSPPFThreadData data;
  data.base = p;
  data.visited = (unsigned char *)alloca(_portal_bytes);
  memset(data.visited, 0, _portal_bytes);
  data.pstack_head.portal = p;
  data.pstack_head.source = p->_winding;
  data.pstack_head.portal_plane = p->_plane;
  data.pstack_head.might_see = (unsigned char *)alloca(_portal_bytes);
  long *pf_long = (long *)p->_portal_flood;
  for (size_t j = 0; j < _portal_longs; j++) {
    ((long *)data.pstack_head.might_see)[j] = pf_long[j];
  }

  assert(count_bits(data.pstack_head.might_see, _portal_list.size()) == p->_num_might_see);

  recursive_leaf_flow(p->_leaf, &data, &data.pstack_head);

  AtomicAdjust::set(p->_status, BSPVisPortal::S_done);

  //if (visbuilder_cat.is_debug()) {
  //  ThreadManager::lock();
  //  std::cerr
  //    << "Portal " << p->_id << ": " << count_bits(p->_portal_vis, _cluster_portals.size()) << " visible portals\n";
  //  ThreadManager::unlock();
  //}
}

/**
 *
 */
void VisBuilderBSP::
recursive_leaf_flow(BSPNode *cluster, BSPPFThreadData *thread, BSPPFStack *prevstack) {
  thread->c_chains++;
  BSPPFStack stack;
  stack.might_see = (unsigned char *)alloca(_portal_bytes);
  prevstack->next = &stack;
  stack.next = nullptr;
  stack.cluster = cluster;
  stack.portal = nullptr;
  stack.num_separators[0] = 0;
  stack.num_separators[1] = 0;

  unsigned char *base_vis = thread->base->_portal_vis;
  LPoint3 base_origin = thread->base->_origin;
  LPlane base_plane = thread->pstack_head.portal_plane;
  PN_stdfloat base_radius = thread->base->_radius;

  long *might = (long *)stack.might_see;
  long *vis = (long *)thread->base->_portal_vis;

  unsigned char *prevmight = prevstack->might_see;
  long *prevmight_long = (long *)prevmight;

  long more;
  long *test;
  int pnum, n;
  BSPVisPortal *p;
  size_t j, i;
  LPlane backplane;
  PN_stdfloat d;

  size_t num_portals = cluster->_vis_portals.size();

  // Check all portals for flowing into other clusters.
  for (i = 0; i < num_portals; i++) {
    p = cluster->_vis_portals[i];
    pnum = p->_id;

    if (!CheckBit(prevmight, pnum)) {
      continue; // Can't possibly see this portal.
    }

    //if (CheckBit(thread->visited, pnum)) {
    //  continue;
    //}

    //SetBit(thread->visited, pnum);

    // If the portal can't see anything we haven't already seen, skip it.
    if (p->_status == BSPVisPortal::S_done) {
      test = (long *)p->_portal_vis;

    } else {
      test = (long *)p->_portal_flood;
    }

    more = 0;
    for (j = 0; j < _portal_longs; j++) {
      might[j] = prevmight_long[j] & test[j];
			more |= (might[j] & ~vis[j]);
    }

    if (!more && CheckBit(base_vis, pnum)) {
      // Can't see anything new.
      continue;
    }

    stack.portal_plane = p->_plane;
    backplane = -(stack.portal_plane);
    stack.portal = p;
    stack.next = nullptr;

    d = base_plane.dist_to_plane(p->_origin);
    if (d < -p->_radius) {
      continue;

    } else if (d > p->_radius) {
      stack.pass = p->_winding;

    } else {
      stack.pass = p->_winding.chop(base_plane);
      //stack.pass = chop_winding(&p->_winding, &stack, base_plane);
      if (stack.pass.is_empty()) {
        continue;
      }
    }

    d = p->_plane.dist_to_plane(base_origin);
    if (d > base_radius) {
      continue;

    } else if (d < -base_radius) {
      stack.source = prevstack->source;

    } else {
      stack.source = prevstack->source.chop(backplane);
      if (stack.source.is_empty()) {
        continue;
      }
      //stack.source = chop_winding(prevstack->source, &stack, backplane);
      //if (stack.source == nullptr || stack.source->is_empty()) {
      //  continue;
      //}
    }

    if (prevstack->pass.is_empty()) {
      // The second cluster can only be blocked if coplanar.
      // Mark the portal as visible
      //std::cout << "\tSet visible " << pnum << "\n";
      SetBit(base_vis, pnum);

      recursive_leaf_flow(p->_leaf, thread, &stack);
      continue;
    }

    if (stack.num_separators[0]) {
      for (n = 0; n < stack.num_separators[0]; n++) {
        stack.pass = stack.pass.chop(stack.separators[0][n]);
        if (stack.pass.is_empty()) {
          break;
        }
        //stack.pass = chop_winding(stack.pass, &stack, stack.separators[0][n]);
        //if (stack.pass == nullptr || stack.pass->is_empty()) {
        //  break; // Target is not visible.
        //}
      }
      if (n < stack.num_separators[0]) {
        continue;
      }
    } else {
      stack.pass = clip_to_seperators(prevstack->source, prevstack->pass, stack.pass, false, &stack);
    }

    if (stack.pass.is_empty()) {
      continue;
    }

    if (stack.num_separators[1]) {
      for (n = 0; n < stack.num_separators[1]; n++) {
        stack.pass = stack.pass.chop(stack.separators[1][n]);
        if (stack.pass.is_empty()) {
          break;
        }
        //stack.pass = chop_winding(stack.pass, &stack, stack.separators[1][n]);
        //if (stack.pass == nullptr || stack.pass->is_empty()) {
        //  break; // Target is not visible.
        //}
      }
    } else {
      stack.pass = clip_to_seperators(prevstack->pass, prevstack->source, stack.pass, true, &stack);
    }

    if (stack.pass.is_empty()) {
      continue;
    }

    //std::cout << "\tSet visible " << pnum << "\n";
    // Mark the portal as visible.
    SetBit(base_vis, pnum);

    // Flow through it for real.
    recursive_leaf_flow(p->_leaf, thread, &stack);
  }
}

/**
 * Source, pass, and target are an ordering of portals.
 *
 * Generates separating planes candidates by taking two points from source and
 * one point from pass, and clips target by them.
 *
 * If target is totally clipped away, that portal cannot be seen through.
 *
 * Normal clip keeps target on the same side as pass, which is correct if the
 * order goes source, pass, target.  If the order goes pass, source, target,
 * then flip_clip should be set.
 */
Winding VisBuilderBSP::
clip_to_seperators(const Winding &source, const Winding &pass,
                   const Winding &target, bool flip_clip,
                   BSPPFStack *stack) {

  LVector3 v1, v2, normal;
  LPlane plane;
  PN_stdfloat length;

  Winding new_target = target;

  // Check all combinations.
  for (int i = 0; i < source.get_num_points(); i++) {
    int l = (i + 1) % source.get_num_points();
    v1 = source.get_point(l) - source.get_point(i);

    // Find a vertex of pass that makes a plane that puts all of the
    // vertices of pass on the front side and all of the vertices of
    // source on the back side.
    int num_pass = pass.get_num_points();
    for (int ipass = 0; ipass < num_pass; ipass++) {
      v2 = pass.get_point(ipass) - source.get_point(i);

      normal = v1.cross(v2);

      // If points don't make a valid plane, skip it.
      length = normal.length_squared();
      if (length < 0.001f) {
        continue;
      }

      normal /= std::sqrt(length);

      plane.set(normal[0], normal[1], normal[2], -(pass.get_point(ipass).dot(normal)));

      // Find out which side of the generated separating plane has the
      // source portal.
      bool flip_test = false;
      int k;
      for (k = 0; k < source.get_num_points(); k++) {
        if (k == i || k == l) {
          continue;
        }
        PN_stdfloat d = plane.dist_to_plane(source.get_point(k));
        if (d < -0.001f) {
          // Source is on the negative side, so we want all pass and target
          // on the positive side.
          flip_test = false;
          break;

        } else if (d > 0.001f) {
          // Source is on the positive side, so we want all pass and target
          // on the negative side.
          flip_test = true;
          break;
        }
      }
      if (k == source.get_num_points()) {
        // Planar with source portal.
        continue;
      }

      // Flip the normal if the source portal is backwards.
      if (flip_test) {
        plane.flip();
      }

      // If all of the pass portal points are now on the positive side,
      // this is the separating plane.
      if (pass.get_plane_side(plane) != PS_front) {
        continue;
      }

      // Flip the normal if we want the back side.
      if (flip_clip) {
        plane.flip();
      }

      stack->separators[flip_clip][stack->num_separators[flip_clip]] = plane;
      if (++stack->num_separators[flip_clip] >= 64) {
        mapbuilder_cat.error()
          << "MAX_SEPARATORS\n";
      }

      // Fast check first
      PN_stdfloat d = plane.dist_to_plane(stack->portal->_origin);
      // If completely at the back of the separator plane
      if (d < -stack->portal->_radius) {
        new_target.clear();
        return new_target;
      }
      // If completely on the front of the separator plane.
      if (d > stack->portal->_radius) {
        break;
      }

      // Clip target by the separating plane.
      new_target = new_target.chop(plane);
      if (new_target.is_empty()) {
        return new_target;
      }
      //target = chop_winding(target, stack, plane);
      //if (target->is_empty()) {
        // Target is not visible.
      //  return target;
      //}

      break;
    }
  }

  return new_target;
}

/**
 * For each cluster, merges vis bits for each portal of cluster onto cluster.
 */
void VisBuilderBSP::
final_leaf_pvs(int i) {
  BSPNode *cluster = _empty_leaf_list[i];
  assert(cluster->_leaf_id >= 0);

  unsigned char *portalvector = (unsigned char *)alloca(_portal_bytes);
  memset(portalvector, 0, _portal_bytes);

  // Merge all portal vis into portalvector for this cluster.
  for (BSPVisPortal *portal : cluster->_vis_portals) {
    for (size_t j = 0; j < _portal_longs; j++) {
      ((long *)portalvector)[j] |= ((long *)portal->_portal_vis)[j];
    }
    SetBit(portalvector, portal->_id);
  }

  // Count the cluster itself.
  cluster->_pvs.insert(cluster->_leaf_id);

  // Now count other visible clusters.
  for (size_t i = 0; i < _portal_list.size(); i++) {
    if (CheckBit(portalvector, _portal_list[i]->_id)) {
      if (_portal_list[i]->_leaf->_leaf_id >= 0) {
        cluster->_pvs.insert(_portal_list[i]->_leaf->_leaf_id);
      }
    }
  }
}

/**
 *
 */
void BSPVisPortal::
calc_radius() {
  Winding *w = &_winding;
  LVector3 total(0);
  for (int i = 0; i < w->get_num_points(); i++) {
    total += w->get_point(i);
  }
  total /= w->get_num_points();

  PN_stdfloat best = 0;
  for (int i = 0; i < w->get_num_points(); i++) {
    LVector3 dist = w->get_point(i) - total;
    PN_stdfloat r = dist.length();
    if (r > best) {
      best = r;
    }
  }

  _radius = best;
}
