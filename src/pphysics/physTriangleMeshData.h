/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriangleMeshData.h
 * @author brian
 * @date 2021-07-09
 */

#ifndef PHYSTRIANGLEMESHDATA_H
#define PHYSTRIANGLEMESHDATA_H

#include "pandabase.h"

#include "referenceCount.h"
#include "physx_includes.h"
#include "physx_utils.h"
#include "pvector.h"

class Geom;
class GeomNode;

/**
 *
 */
class EXPCL_PANDA_PPHYSICS PhysTriangleMeshData : public ReferenceCount {
PUBLISHED:
  INLINE PhysTriangleMeshData();
  INLINE PhysTriangleMeshData(CPTA_uchar mesh_data);

  void add_triangle(const LPoint3 &v1, const LPoint3 &v2, const LPoint3 &v3, int material_index = 0);
  void add_quad(const LPoint3 &v1, const LPoint3 &v2, const LPoint3 &v3, const LPoint3 &v4, int material_index = 0);
  void add_polygon(const pvector<LPoint3> &vertices, int material_index = 0);

  void add_triangles_from_geom(const Geom *geom, const LMatrix4 &mat = LMatrix4::ident_mat(), int material_index = 0);
  void add_triangles_from_geom_node(GeomNode *node, bool world_space = false, int material_index = 0);

  void add_vertices(const pvector<LPoint3> &vertices);
  void add_triangle_indices(int v0, int v1, int v2, int material_index = 0);

  INLINE size_t get_num_vertices() const;
  INLINE LPoint3 get_vertex(size_t n) const;

  INLINE size_t get_num_indices() const;
  INLINE size_t get_index(size_t n) const;

  INLINE void clear_triangles();

  INLINE bool has_mesh() const;
  INLINE bool has_mesh_data() const;

  INLINE void invalidate_mesh();

  bool generate_mesh();
  bool cook_mesh();

  INLINE CPTA_uchar get_mesh_data() const;

public:
  INLINE physx::PxTriangleMesh *get_mesh() const;

private:
  pvector<physx::PxVec3> _vertices;
  pvector<physx::PxU32> _indices;
  pvector<physx::PxMaterialTableIndex> _mat_indices;

  physx::PxTriangleMesh *_mesh;
  CPTA_uchar _mesh_data;
  bool _has_mesh_data;
};

#include "physTriangleMeshData.I"

#endif // PHYSTRIANGLEMESHDATA_H
