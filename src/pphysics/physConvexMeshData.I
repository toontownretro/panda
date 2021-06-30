/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physConvexMeshData.I
 * @author brian
 * @date 2021-04-22
 */

/**
 *
 */
INLINE PhysConvexMeshData::
PhysConvexMeshData() :
  _mesh(nullptr),
  _has_mesh_data(false)
{
}

/**
 *
 */
INLINE PhysConvexMeshData::
PhysConvexMeshData(CPTA_uchar mesh_data) :
  _mesh(nullptr),
  _mesh_data(mesh_data),
  _has_mesh_data(true)
{
}

/**
 *
 */
INLINE void PhysConvexMeshData::
add_point(const LPoint3 &point) {
  _points.push_back(panda_vec_to_physx(point));
  invalidate_mesh();
}

/**
 *
 */
INLINE size_t PhysConvexMeshData::
get_num_points() const {
  return _points.size();
}

/**
 *
 */
INLINE LPoint3 PhysConvexMeshData::
get_point(size_t n) const {
  return physx_vec_to_panda(_points[n]);
}

/**
 *
 */
INLINE void PhysConvexMeshData::
clear_points() {
  _points.clear();
  invalidate_mesh();
}

/**
 * Returns true if the object contains an actual convex mesh object.  This will
 * return true if generate_mesh() was called with cook being false, and a list
 * of input points were filled in on the object.
 */
INLINE bool PhysConvexMeshData::
has_mesh() const {
  return _mesh != nullptr;
}

/**
 * Returns true if the object contains a buffer of convex mesh data.  This will
 * return true if the object was initialized with a buffer, or generate_mesh()
 * was called with cook being true.
 */
INLINE bool PhysConvexMeshData::
has_mesh_data() const {
  return _has_mesh_data;
}

/**
 *
 */
INLINE void PhysConvexMeshData::
invalidate_mesh() {
  if (_mesh != nullptr) {
    _mesh->release();
    _mesh = nullptr;
  }
}

/**
 * Returns the buffer containing the convex mesh data.  This buffer will only
 * exist if the object was initialized with a buffer, or generate_mesh() was
 * called with cook being true.
 */
INLINE CPTA_uchar PhysConvexMeshData::
get_mesh_data() const {
  return _mesh_data;
}

/**
 *
 */
INLINE physx::PxConvexMesh *PhysConvexMeshData::
get_mesh() const {
  return _mesh;
}
