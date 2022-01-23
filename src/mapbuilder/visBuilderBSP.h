/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file visBuilderBSP.h
 * @author brian
 * @date 2021-12-21
 */

#ifndef VISBUILDERBSP_H
#define VISBUILDERBSP_H

#include "pandabase.h"
#include "bspTree.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "winding.h"
#include "boundingBox.h"
#include "atomicAdjust.h"
#include "bitArray.h"
#include "pset.h"

typedef BaseWinding<12> BSPPortalWinding;

class BSPNode;
class MapBuilder;
class BSPPortal;

/**
 * An input polygon to the BSP tree.  These perform BSP tree cuts and
 * contribute to visibilty cell generation.  Opaque faces block visibility,
 * hints just perform extra BSP cuts but don't block visibility and result
 * in portals.
 */
class BSPFace : public ReferenceCount {
public:
  Winding _winding;
  bool _hint = false;
  int _priority = 0;
  unsigned int _contents = 0;
  bool _visible = false;
  bool _checked = false;
};
typedef pvector<PT(BSPFace)> BSPFaces;

/**
 *
 */
class BSPSolid : public ReferenceCount {
public:
  BSPFaces _faces;
  bool _opaque;

  void clip(const LPlane &plane, PT(BSPSolid) &front, PT(BSPSolid) &back) const;
};

/**
 *
 */
class BSPPortal : public ReferenceCount {
public:
  BSPPortal() = default;

  // Polygon of the portal.
  Winding _winding;
  LPlane _plane;

  BSPNode *_nodes[2] = { nullptr, nullptr };

  BSPNode *_on_node = nullptr;

  bool _hint = false;

  size_t _id = 0;
};

/**
 * Portal representation for the PVS computation.
 * Two BSPVisPortals exist for the front-side and back-side
 * of a single BSPPortal.
 */
class BSPVisPortal : public ReferenceCount {
public:
  enum Status {
    S_none,
    S_working,
    S_done,
  };

  BSPVisPortal() = default;

  Winding _winding;
  LPlane _plane;

  LPoint3 _origin;
  PN_stdfloat _radius = 0.0f;

  size_t _id = 0;

  // Specific to PVS pass.
  unsigned char *_portal_front = nullptr;
  unsigned char *_portal_flood = nullptr;
  unsigned char *_portal_vis = nullptr;
  int _num_might_see = 0;
  AtomicAdjust::Integer _status = S_none;

  bool _hint = false;

  BSPNode *_leaf;

  void calc_radius();
};

class BSPPFStack {
public:
  BSPPFStack() {
    next = nullptr;
    cluster = nullptr;
    portal = nullptr;
    //source = pass = nullptr;
    might_see = nullptr;
    num_separators[0] = num_separators[1] = 0;
  }

  unsigned char *might_see;
  BSPPFStack *next;
  BSPNode *cluster;
  BSPVisPortal *portal;
  Winding source, pass;

  //Winding windings[3];

  LPlane portal_plane;

  LPlane separators[2][64];
  int num_separators[2];
};

class BSPPFThreadData {
public:
  BSPPFThreadData() {
    base = nullptr;
    visited = nullptr;
    c_chains = 0;
  }

  BSPVisPortal *base;
  int c_chains;
  unsigned char *visited;
  BSPPFStack pstack_head;
};

/**
 * A single node of the BSP tree constructed and used during the bake.
 * This is stored in a way that makes it more convenient for use while baking.
 * A more efficient version of the tree is written to the Bam file and queried
 * at runtime.
 */
class BSPNode : public ReferenceCount {
public:
  BSPNode() = default;

  BSPNode *_parent = nullptr;

  // True if the node was formed by a hint split and not a polygon.
  bool _hint = false;

  bool _opaque = false;

  // True if a path through portals exists from an entity to this node.
  // If false, the node is unreachable and will be filled.
  bool _occupied = false;

  // Both nodes and leaves have portal lists.  Nodes have portals during portal
  // generation, but are moved down to the leaves at the end.
  pvector<PT(BSPPortal)> _portals;
  pvector<BSPVisPortal *> _vis_portals;
  pset<int> _pvs;
  int _leaf_index = -1;
  // Not -1 if empty leaf.
  int _leaf_id = -1;

  LPlane _plane = LPlane(0.0f, 1.0f, 0.0f, 0.0f);
  PT(BSPNode) _children[2] = { nullptr, nullptr };

  LPoint3 _mins, _maxs;

  // The node is a leaf if it has no children at all.
  INLINE bool is_leaf() const { return _children[0] == nullptr && _children[1] == nullptr; }
};

/**
 * This is a VisBuilder variant that computes visibility information by
 * building a solid-leaf BSP tree from the input occluder polygons.  Leaf
 * nodes of the BSP tree represent visibility cells.  Portals are generated
 * between adjacent visibility cells and the potentially visible set is
 * calculated from there.
 *
 * This is the well-established method for baking visibility information into
 * game levels since the Quake days.
 *
 * Advantages:
 * - Visibility cells conform precisely to the geometry that formed the cell.
 * - Visibility cells are convex sub-spaces instead of just axis-aligned boxes.
 * - Uses significantly less memory than voxels.
 * - Faster to compute.
 * - BSP splits can be assisted by user-created "hint" splitting planes.
 *
 * Disadvantages:
 * - Complexity of visibility cells is bound to the complexity of the occluder
 *   geometry.  Every occluder polygon splits the BSP tree.
 * - Occluder geometry must be watertight sealed.
 *
 * The difference between this and the Quake method is that occluder polygons
 * that cross a BSP split plane are simply added to both sides instead of split
 * into two polygons.  The BSP tree is only used for building visibility cells,
 * not rendering.
 *
 * Quake III Arena BSP code used as reference.
 */
class VisBuilderBSP {
public:
  bool bake();

private:
  bool build_bsp_tree();
  bool build_portals();
  bool calc_pvs();

  bool build_output_tree();
  int r_build_output_tree(const BSPNode *node, int parent);

  void make_subtree(BSPNode *node, const BSPFaces &faces);
  int pick_best_split(const BSPFaces &faces);

  void make_headnode_portals();
  void r_make_tree_portals(BSPNode *node);
  void make_node_portal(BSPNode *node);
  void split_node_portals(BSPNode *node);

  void add_portal_to_nodes(BSPPortal *portal, BSPNode *front, BSPNode *back);
  void remove_portal_from_node(BSPPortal *portal, BSPNode *node);

  void calc_node_portal_bounds(BSPNode *node);

  Winding get_node_winding(BSPNode *node);

  bool flood_entities();
  bool place_occupant(BSPNode *node, const LPoint3 &origin);
  void r_flood_portals(BSPNode *node);
  void r_fill_outside(BSPNode *node);
  void r_remove_opaque_portals(BSPNode *node);

  void filter_structural_solids_into_tree();
  int r_filter_structural_solid_into_tree(BSPSolid *solid, BSPNode *node);

  void mark_visible_sides();
  void r_mark_visible_sides(BSPFace *side, Winding winding, BSPNode *node);

  void r_build_portal_list(BSPNode *node);

  void sort_portals();
  void portal_flow(int i);
  void recursive_leaf_flow(BSPNode *node, BSPPFThreadData *data, BSPPFStack *stack);
  void base_portal_vis(int i);
  void simple_flood(BSPVisPortal *portal, BSPNode *node);
  void final_leaf_pvs(int i);
  Winding clip_to_seperators(const Winding &source, const Winding &pass,
                             const Winding &target, bool flip_clip, BSPPFStack *stack);


public:
  PT(BSPNode) _tree_root;
  PT(BSPNode) _outside_node;

  MapBuilder *_builder;

  BSPFaces _input_faces;
  pvector<PT(BSPSolid)> _input_solids;

  pvector<PT(BSPVisPortal)> _portal_list;
  pvector<BSPVisPortal *> _sorted_portals;

  pvector<BSPNode *> _leaf_list;
  pvector<BSPNode *> _empty_leaf_list;

  PT(BSPTree) _output_tree;

  bool _hint_split;

  size_t _portal_bytes;
  size_t _portal_longs;
};

#include "visBuilderBSP.I"

#endif // VISBUILDERBSP_H
