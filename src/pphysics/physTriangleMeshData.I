/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physTriangleMeshData.I
 * @author brian
 * @date 2021-07-09
 */

/**
 *
 */
INLINE PhysTriangleMeshData::
PhysTriangleMeshData() :
  _mesh(nullptr),
  _has_mesh_data(false)
{
}

/**
 *
 */
INLINE PhysTriangleMeshData::
PhysTriangleMeshData(CPTA_uchar mesh_data) :
  _mesh(nullptr),
  _mesh_data(mesh_data),
  _has_mesh_data(true)
{
}

/**
 *
 */
INLINE size_t PhysTriangleMeshData::
get_num_vertices() const {
  return _vertices.size();
}

/**
 *
 */
INLINE LPoint3 PhysTriangleMeshData::
get_vertex(size_t n) const {
  nassertr(n >= 0 && n < _vertices.size(), LPoint3());
  return physx_vec_to_panda(_vertices[n]);
}

/**
 *
 */
INLINE size_t PhysTriangleMeshData::
get_num_indices() const {
  return _indices.size();
}

/**
 *
 */
INLINE size_t PhysTriangleMeshData::
get_index(size_t n) const {
  nassertr(n >= 0 && n < _indices.size(), 0);
  return _indices[n];
}

/**
 *
 */
INLINE void PhysTriangleMeshData::
clear_triangles() {
  _vertices.clear();
  _indices.clear();
  invalidate_mesh();
}

/**
 *
 */
INLINE bool PhysTriangleMeshData::
has_mesh() const {
  return _mesh != nullptr;
}

/**
 *
 */
INLINE bool PhysTriangleMeshData::
has_mesh_data() const {
  return _has_mesh_data;
}

/**
 *
 */
INLINE void PhysTriangleMeshData::
invalidate_mesh() {
  if (_mesh != nullptr) {
    _mesh->release();
    _mesh = nullptr;
  }
}

/**
 *
 */
INLINE CPTA_uchar PhysTriangleMeshData::
get_mesh_data() const {
  return _mesh_data;
}

/**
 *
 */
INLINE physx::PxTriangleMesh *PhysTriangleMeshData::
get_mesh() const {
  return _mesh;
}
