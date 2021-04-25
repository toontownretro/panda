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
  _mesh(nullptr)
{
}

/**
 *
 */
INLINE void PhysConvexMeshData::
add_point(const LPoint3 &point) {
  _points.push_back(Vec3_to_PxVec3(point));
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
  return PxVec3_to_Vec3(_points[n]);
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
 *
 */
INLINE bool PhysConvexMeshData::
has_mesh() const {
  return _mesh != nullptr;
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
 *
 */
INLINE physx::PxConvexMesh *PhysConvexMeshData::
get_mesh() const {
  return _mesh;
}
