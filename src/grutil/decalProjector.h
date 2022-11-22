/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decalProjector.h
 * @author brian
 * @date 2022-11-05
 */

#ifndef DECALPROJECTOR_H
#define DECALPROJECTOR_H

#include "pandabase.h"
#include "nodePath.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "geom.h"
#include "winding.h"
#include "luse.h"
#include "plane.h"
#include "geomTriangles.h"
#include "geomVertexData.h"
#include "geom.h"
#include "boundingBox.h"
#include "pmap.h"
#include "pointerTo.h"
#include "geomTriangleOctree.h"
#include "geomVertexReader.h"
#include "pset.h"
#include "lightMutex.h"

// There are multiple coordinate spaces we're dealing with
// here.
//
// 1) Projector-space.  This is the coordinate space of the
//    decal projector.  The user-specified location of the
//    decal.
// 2) Decal-space.  The coordinate space of the generated decal
//    geometry.
//
// 3) Mesh-space.  The coordinate space of the mesh being decaled.
//
// We perform clipping in world-space.  The projector and meshes being
// decaled are moved into world-space.  When generating geometry, we
// transform the clipped triangles by the inverse of the decal coordinate
// space.
//

typedef BaseWinding<9> DecalWinding;

/**
 *
 */
class EXPCL_PANDA_GRUTIL DecalProjector {
private:
  class Readers {
  public:
    GeomVertexReader _vertex;
    GeomVertexReader _normal;
    GeomVertexReader _tangent;
    GeomVertexReader _binormal;
  };

PUBLISHED:
  INLINE DecalProjector();
  virtual ~DecalProjector() = default;

  // Associated with the decal projector.
  INLINE void set_projector_parent(const NodePath &parent);
  INLINE void set_projector_transform(const TransformState *transform);
  INLINE void set_projector_bounds(const LPoint3 &mins, const LPoint3 &maxs);

  // Associated with the generated decal geometry.
  INLINE void set_decal_parent(const NodePath &parent);
  INLINE void set_decal_uv_transform(const TransformState *state);
  INLINE void set_decal_render_state(const RenderState *state);

  bool project(const NodePath &root);
  bool project(GeomNode *geom_node, const TransformState *net_transform);
  bool project(const Geom *geom, const TransformState *net_transform);
  ALWAYS_INLINE bool project(const Readers &readers, int v1, int v2, int v3, const TransformState *net_transform);

  INLINE bool box_overlap(const LPoint3 &min_a, const LPoint3 &max_a, const LPoint3 &min_b, const LPoint3 &max_b) const;

  void setup_coordinate_space();

  void clear();

  PT(PandaNode) generate();

  static void set_geom_octree(const Geom *geom, GeomTriangleOctree *octree);
  static void clear_geom_octree(const Geom *geom);
  static void clear_geom_octrees();

private:
  bool r_project(PandaNode *node, const TransformState *net_transform);

  bool r_project_octree(const Readers &readers, const GeomTriangleOctree::OctreeNode *node,
                        const TransformState *net_transform, const BoundingBox *projector_bbox,
                        pset<int> &clipped_triangles,
                        const GeomTriangleOctree *tree);

  LVecBase3 calc_barycentric_coordinates(const LPoint3 &a, const LPoint3 &b,
                                         const LPoint3 &c, const LPoint3 &pos) const;

private:
  class DecalFragment {
  public:
    class OrigVertex {
    public:
      LPoint3 _pos;
      LVecBase2 _texcoord_lightmap;
      LVector3 _normal;
      LVector3 _tangent;
      LVector3 _binormal;
    };
    OrigVertex _orig_vertices[3];

    DecalWinding _winding;
  };
  pvector<DecalFragment> _fragments;
  LightMutex _fragments_lock;

  // Defines the coordinate-space of the projector.
  NodePath _projector_parent;
  CPT(TransformState) _projector_transform;
  // Relative to _projector_transform.
  LPoint3 _projector_mins, _projector_maxs;

  // Defines the coordinate-space of the generated decal
  // geometry.
  NodePath _decal_parent;
  CPT(RenderState) _decal_state;
  CPT(TransformState) _decal_uv_transform;

  PT(BoundingBox) _projector_world_bbox;

  // World-space plane for each face of the projector bounding box.
  LPlane _box_planes[6];
  LVector3 _projector_world_forward;

  LMatrix4 _decal_inv_net_mat;
  LMatrix4 _projector_inv_net_mat;

  LPoint3 _projector_world_center;
  LPoint3 _projector_world_extents;

  typedef pflat_hash_map<const Geom *, PT(GeomTriangleOctree), pointer_hash> GeomOctrees;
  static GeomOctrees _octrees;

  int _num_vertices;
  int _num_indices;
};

#include "decalProjector.I"

#endif // DECALPROJECTOR_H
